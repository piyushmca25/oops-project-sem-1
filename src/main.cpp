#include <iostream> // for debug printing if needed
#include <raylib.h>  // core raylib API (windowing, drawing, input, textures, sounds)
#include <deque>     // double-ended queue used to store snake body segments
#include <raymath.h> // raymath provides Vector2 helpers like Vector2Add and equality
#include "button.hpp" // custom button helper (assumed to exist)
#include <vector>    // dynamic array used for fruits

using namespace std;

/*
 * Global color definitions
 * OBJECTIVE: Reusable colors for the game's UI and drawing.
 * SIDE EFFECTS: None, purely cosmetic values used by drawing commands.
 */
Color green = {173, 204, 96, 255};     // background color used for the main playing area
Color darkGreen = {43, 51, 24, 255};  // used for borders, snake segments and text

/*
 * Layout and timing globals
 * OBJECTIVE: control the grid size, speed and offsets for drawing.
 * SIDE EFFECTS: these values affect drawing scale and game behavior globally.
 */
int cellsize = 30;    // size in pixels of a single grid cell
int cellcount = 25;   // number of cells per row/column (square grid)
double lastUpdateTime = 0; // timestamp of the last automatic update; used for throttling movement
int offset = 75;      // pixel offset from the window edge to the top-left corner of the grid
int temp_score;       // temporary holder for last game score (set on game over)
int high_score = 0;   // persisted high score for the current program run

/*
 * ElementInDeque
 * OBJECTIVE: Check whether a given Vector2 (cell) exists within a deque of Vector2.
 * INPUT : Vector2 element - the cell to search for
 *        deque<Vector2> deque - the container to search in (passed by value in original code)
 * OUTPUT: none
 * RETURN VALUE: bool - true if the element exists in the deque, false otherwise
 * SIDE EFFECTS: none (pure query)
 *
 * APPROACH:
 * Iterate through all elements of the deque and compare each with the target
 * using Vector2Equals from raymath.h. Early return on match.
 *
 * Variable definition and use:
 * i - index used for iteration.
 */
bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    // loop through each element in the supplied deque
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        // compare using raylib's Vector2Equals which checks x and y with float tolerance
        if (Vector2Equals(deque[i], element))
        {
            return true; // found the element
        }
    }
    return false; // not found after checking all entries
}

/*
 * EventTriggered
 * OBJECTIVE: determine whether the given time interval has elapsed since
 *            the last recorded update (lastUpdateTime) and, if so, update
 *            lastUpdateTime and return true.
 * INPUT : double interval - minimum seconds between events
 * OUTPUT: none
 * RETURN VALUE: bool - true if enough time has passed; false otherwise
 * SIDE EFFECTS: updates global lastUpdateTime when returning true; affects game timing
 *
 * APPROACH:
 * Compare the current time (GetTime) against lastUpdateTime. If difference >= interval,
 * set lastUpdateTime to current time and return true; otherwise return false.
 *
 * VARIABLE DEFINITION AND USE:
 * currentTime - fetched from raylib's GetTime() which returns seconds as double.
 */
bool EventTriggered(double interval)
{
    double currentTime = GetTime(); // get current time in seconds since initialization
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime; // update the last triggered timestamp
        return true;                 // signal that the event should be processed
    }
    return false; // not enough time has passed yet
}

/*
 * Snake class
 * OBJECTIVE: encapsulates the snake's state and operations (draw, update, reset).
 * MEMBER VARIABLES:
 *  - addSegment : when true, the snake will grow by one segment on next update.
 *  - body       : deque of Vector2 representing the grid cells occupied by the snake.
 *  - direction  : unit Vector2 indicating the current movement direction (e.g., {1,0}).
 * MEMBER FUNCTIONS:
 *  - Draw()     : draws all segments of the snake.
 *  - Update()   : advances the snake by one cell in the current direction.
 *  - Reset()    : restores initial position and direction.
 */
class Snake
{
public:
    bool addSegment = false; // when true, do not pop back on next Update() so snake grows
    deque<Vector2> body = {{6, 9}, {5, 9}, {4, 9}}; // initial 3-segment snake placed on grid
    Vector2 direction = {1, 0}; // initial movement direction = right

    /*
     * Draw
     * OBJECTIVE: render each segment of the snake onto the screen.
     * INPUT : none (uses member variables and global drawing context)
     * OUTPUT: draws rectangles to the active RenderTexture/Screen
     * RETURN VALUE: void
     * SIDE EFFECTS: performs drawing operations which depend on raylib BeginDrawing/EndDrawing
     *
     * APPROACH:
     * Iterate through body deque and draw a rounded rectangle for each cell.
     *
     * VARIABLE DEFINITION AND USE:
     * x, y - coordinates of segment
     * segment - Rectangle used for DrawRectangleRounded
     */
    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x; // grid x coordinate of this segment
            float y = body[i].y; // grid y coordinate of this segment

            // compute pixel-space rectangle to draw for this segment using global offset & cellsize
            Rectangle segment = Rectangle{offset + x * cellsize, offset + y * cellsize, (float)cellsize, (float)cellsize};

            // draw a rounded rectangle for the segment with a fixed roundness and corner segments
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    /*
     * Update
     * OBJECTIVE: move the snake one cell in the current direction, and optionally grow.
     * INPUT : none (uses member variables)
     * OUTPUT: modifies the body deque
     * RETURN VALUE: void
     * SIDE EFFECTS: mutates snake.body and addSegment
     *
     * APPROACH:
     * Push a new head position equal to head + direction; if addSegment is true, leave the
     * tail so the snake grows; otherwise pop the tail to keep length constant.
     *
     * VARIABLE DEFINITION AND USE:
     * (no extra locals)
     */
    void Update()
    {
        // add a new head at the current head position plus the direction vector
        body.push_front(Vector2Add(body[0], direction));

        if (addSegment)
        {
            addSegment = false; // growth applied; reset flag so growth happens only once per food
        }
        else
        {
            body.pop_back(); // remove last element to keep the snake the same length
        }
    }

    /*
     * Reset
     * OBJECTIVE: restore the snake to its initial starting configuration.
     * INPUT : none
     * OUTPUT: resets member variables body and direction
     * RETURN VALUE: void
     * SIDE EFFECTS: mutates snake state used by game loop
     *
     * APPROACH: assign initial literal values for body and direction.
     */
    void Reset()
    {
        // set the snake to the original three cells and facing right
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

/*
 * Food class
 * OBJECTIVE: represent a food item that the snake can eat. Manages textures for different
 *            food visuals and generates positions that do not collide with the snake.
 *
 * STATIC MEMBERS:
 *  - textures[] : array of 4 textures shared by all Food instances to avoid reloading.
 *  - loaded     : flag indicating static textures have been loaded.
 *
 * INSTANCE MEMBERS:
 *  - position   : grid cell where this food is located
 *  - textureIndex : which texture to draw from textures[]
 */
class Food
{
public:
    static Texture2D textures[4]; // shared textures for all Food objects
    static bool loaded;           // whether textures[] are loaded
    Vector2 position;             // current grid cell for this fruit
    int textureIndex;             // index into textures[] to select visual

    /*
     * CONSTRUCTOR
     * OBJECTIVE: initialize the food instance, load static textures if needed and
     *            pick a random free position not occupied by the snake.
     * INPUT : deque<Vector2> snakeBody - current snake body to avoid placing food on it
     * OUTPUT: sets position and textureIndex. On first call may load textures from disk.
     * RETURN VALUE: none
     * SIDE EFFECTS: loads and stores static textures; uses raylib file IO so failures affect program
     *
     * APPROACH:
     * If textures are not yet loaded, load four images from disk and convert to textures.
     * Choose a random texture index (0..3) and generate a random valid position.
     *
     * VARIABLE DEFINITION AND USE:
     * img1..img4 - temporary Image objects used to create textures. They must be Unloaded after conversion.
     */
    Food(deque<Vector2> snakeBody)
    {
        if (!loaded)
        {
            // load images and convert to textures only once to save memory and IO
            Image img1 = LoadImage("graphics/food1.png");
            Image img2 = LoadImage("graphics/food2.png");
            Image img3 = LoadImage("graphics/food3.png");
            Image img4 = LoadImage("graphics/food4.png");

            textures[0] = LoadTextureFromImage(img1);
            textures[1] = LoadTextureFromImage(img2);
            textures[2] = LoadTextureFromImage(img3);
            textures[3] = LoadTextureFromImage(img4);

            // free temporary image memory; textures remain in GPU memory
            UnloadImage(img1);
            UnloadImage(img2);
            UnloadImage(img3);
            UnloadImage(img4);

            loaded = true; // mark static textures as ready
        }

        // pick a random visual for this food instance
        textureIndex = GetRandomValue(0, 3);

        // find a grid cell not occupied by the snake
        position = GenerateRandomPos(snakeBody);
    }

    /*
     * GenerateRandomCell
     * OBJECTIVE: return a single random cell coordinate inside the grid bounds.
     * INPUT : none
     * OUTPUT: none
     * RETURN VALUE: Vector2 containing x and y integers represented as floats
     * SIDE EFFECTS: none
     *
     * APPROACH: call GetRandomValue for both x and y within [0, cellcount-1]
     */
    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellcount - 1); // integer-like float
        float y = GetRandomValue(0, cellcount - 1);
        return Vector2{x, y}; // return new Vector2
    }

    /*
     * GenerateRandomPos
     * OBJECTIVE: pick a random cell that is not part of snakeBody.
     * INPUT : deque<Vector2> snakeBody - cells to avoid
     * OUTPUT: none
     * RETURN VALUE: Vector2 valid cell for food
     * SIDE EFFECTS: none
     *
     * APPROACH: repeatedly call GenerateRandomCell until the chosen cell is not
     * present in snakeBody using ElementInDeque.
     *
     * VARIABLE DEFINITION AND USE:
     * newPos - candidate position which is regenerated while it collides with snake.
     */
    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 newPos = GenerateRandomCell();
        while (ElementInDeque(newPos, snakeBody))
        {
            newPos = GenerateRandomCell(); // try again until free cell found
        }
        return newPos;
    }

    /*
     * Draw
     * OBJECTIVE: draw the food texture at its grid position.
     * INPUT : none
     * OUTPUT: draws texture to the current render target
     * RETURN VALUE: void
     * SIDE EFFECTS: uses raylib DrawTexture which requires a valid texture
     *
     * APPROACH: convert grid position to pixel coordinates using offset and cellsize
     */
    void Draw()
    {
        DrawTexture(
            textures[textureIndex], // selected texture
            offset + position.x * cellsize, // compute x pixel
            offset + position.y * cellsize, // compute y pixel
            WHITE); // tint color white => original texture colors
    }
};

// initialize static members for Food
Texture2D Food::textures[4] = {0};
bool Food::loaded = false;

/*
 * Game class
 * OBJECTIVE: manage the overall game state, input handling, collisions, scoring and audio.
 * MEMBER VARIABLES:
 *  - score : current score for the active round
 *  - speed : interval (in seconds) between automatic game updates (snake movements)
 *  - running : whether the game simulation is currently running
 *  - game_over : whether the last round ended
 *  - snake : Snake instance tracking snake body and movement
 *  - fruits : vector of Food objects present on the board
 *  - wall, eat : Sound objects for audio feedback
 *
 * MEMBER FUNCTIONS:
 *  - constructor: loads sounds, initializes fruits and audio device
 *  - destructor: unloads textures & sounds and closes audio device
 *  - Draw: delegates drawing to snake and all fruits
 *  - CheckCollisionWithFood: test and handle snake eating fruit
 *  - CheckCollisionWithEdges: handle when snake crosses outside bounds
 *  - CheckCollisionsWithTail: handle self-collision
 *  - Update: perform one game tick (move snake and run collision checks)
 *  - GameOver: handle end-of-round cleanup and score reset
 */
class Game
{
public:
    int score = 0;      // running score for the current play
    double speed = 0.2; // initial movement interval in seconds
    bool running = false; // whether the simulation is active
    bool game_over = false; // whether we are currently in a game-over state
    Snake snake = Snake(); // the player's snake instance
    vector<Food> fruits;   // currently active food objects on the board
    Sound wall;            // sound to play on collision
    Sound eat;             // sound to play when eating food

    /*
     * Constructor
     * OBJECTIVE: initialize audio subsystem, load sounds and populate initial fruits.
     * SIDE EFFECTS: allocates audio resources and loads files from disk (may fail on missing files)
     *
     * APPROACH: call InitAudioDevice, load sound files and create N Food objects while avoiding the snake.
     */
    Game()
    {
        InitAudioDevice(); // start audio system for playback

        // load sound files for wall collision and eating; paths are relative to executable
        wall = LoadSound("sounds/wall.mp3");
        eat = LoadSound("sounds/eat.mp3");

        int fruitCount = 3; // number of fruits to maintain concurrently
        for (int i = 0; i < fruitCount; i++)
        {
            fruits.push_back(Food(snake.body)); // spawn fruit avoiding current snake body
        }
    }

    /*
     * Destructor
     * OBJECTIVE: release GPU textures and audio resources when Game object is destroyed.
     * SIDE EFFECTS: unloading textures and closing audio device affects other audio code
     *
     * APPROACH: unload each of the shared Food textures (4), unload sounds and close audio.
     */
    ~Game()
    {
        for (int i = 0; i < 4; i++)
            UnloadTexture(Food::textures[i]); // free GPU texture memory
        UnloadSound(eat); // free sound resources
        UnloadSound(wall);
        CloseAudioDevice(); // shutdown audio
    }

    /*
     * Draw
     * OBJECTIVE: draw the snake and all fruits to the screen.
     * INPUT: none
     * OUTPUT: draws objects via raylib
     * RETURN VALUE: void
     * SIDE EFFECTS: renders to screen
     */
    void Draw()
    {
        snake.Draw(); // draw the player's snake
        for (auto &f : fruits)
            f.Draw(); // draw each fruit
    }

    /*
     * CheckCollisionWithFood
     * OBJECTIVE: detect when the snake head occupies the same cell as a fruit,
     *            handle eating (increase score, grow snake, respawn fruit, play sound).
     * INPUT: none (uses members)
     * OUTPUT: updates score, snake.addSegment, fruit position and textureIndex
     * RETURN VALUE: void
     * SIDE EFFECTS: mutates fruits and snake state, plays sound, adjusts speed
     *
     * APPROACH: iterate over fruits; if head equals fruit.position, move the fruit
     * to a new valid location, mark snake to grow, increment score and optionally speed up.
     */
    void CheckCollisionWithFood()
    {
        for (auto &f : fruits)
        {
            if (Vector2Equals(snake.body[0], f.position)) // head equals fruit
            {
                f.position = f.GenerateRandomPos(snake.body); // respawn fruit
                f.textureIndex = GetRandomValue(0, 3); // randomize appearance
                snake.addSegment = true; // cause growth on next update
                score++; // increase score
                if(speed>=0.07)
                    speed *= 0.98; // slightly increase speed by reducing interval
                PlaySound(eat); // play eating sound
            }
        }
    }

    /*
     * CheckCollisionWithEdges
     * OBJECTIVE: detect when the snake head moves beyond the grid and trigger game over.
     * INPUT: none
     * OUTPUT: may set game_over true and call GameOver()
     * RETURN VALUE: void
     * SIDE EFFECTS: can reset game state and play sound
     *
     * APPROACH: compare head x/y against grid bounds [0, cellcount-1]; if outside trigger GameOver
     */
    void CheckCollisionWithEdges()
    {
        // check horizontal bounds (if head's x equals cellcount it's beyond right edge; -1 is beyond left)
        if (snake.body[0].x == cellcount || snake.body[0].x == -1)
        {
            GameOver(); // handle game over state
            PlaySound(wall); // play collision sound
        }
        // check vertical bounds (beyond bottom or top)
        if (snake.body[0].y == cellcount || snake.body[0].y == -1)
        {
            GameOver();
            PlaySound(wall);
        }
    }

    /*
     * CheckCollisionsWithTail
     * OBJECTIVE: detect self-collision when head overlaps any other body segment and trigger game over.
     * INPUT: none
     * OUTPUT: may set game_over true and call GameOver()
     * RETURN VALUE: void
     * SIDE EFFECTS: resets game state and plays collision sound
     *
     * APPROACH: copy body to temporary 'headless' deque, remove the head and check if head equals any element.
     * This avoids comparing the head with itself.
     */
    void CheckCollisionsWithTail()
    {
        deque<Vector2> headless = snake.body; // make a copy of the body
        headless.pop_front(); // remove head so we only check against tail segments
        if (ElementInDeque(snake.body[0], headless))
        {
            GameOver(); // collided with tail
            PlaySound(wall);
        }
    }

    /*
     * Update
     * OBJECTIVE: advance game simulation by one tick if running: move snake and run collision checks.
     * INPUT: none
     * OUTPUT: updates snake and game state
     * RETURN VALUE: void
     * SIDE EFFECTS: calls snake.Update which mutates its body
     */
    void Update()
    {
        if (running)
        {
            snake.Update(); // move the snake forward
            CheckCollisionWithFood(); // handle eating
            CheckCollisionWithEdges(); // handle boundary collision
            CheckCollisionsWithTail(); // handle self collision
        }
    }

    /*
     * GameOver
     * OBJECTIVE: perform end-of-round tasks: reset snake and fruits, adjust speed and track high score.
     * INPUT: none
     * OUTPUT: resets game members
     * RETURN VALUE: void
     * SIDE EFFECTS: modifies global high_score and temp_score; resets game to waiting state
     *
     * APPROACH: set game_over flag and reset snake; clear and repopulate fruits; update high_score logic.
     */
    void GameOver()
    {
        game_over = true; // enter game over state
        snake.Reset(); // reset snake to starting position
        fruits.clear(); // remove all fruits
        for (int i = 0; i < 3; i++)
        {
            fruits.push_back(Food(snake.body)); // respawn three fruits at safe positions
        }
        speed = 0.2; // restore initial speed
        running = false; // stop simulation
        if (score >= high_score)
        {
            high_score = score; // update high score if needed
        }
        temp_score = score; // copy last score for display on game over screen
        score = 0; // reset current score
    }
};

/*
 * main
 * OBJECTIVE: initialize the window, create UI buttons and run the main game loop handling input,
 *            drawing and game state transitions.
 * INPUT: none
 * OUTPUT: runs the application window until closed
 * RETURN VALUE: int - 0 on normal exit
 * SIDE EFFECTS: opens window and audio device; loads assets via Game and Button constructors
 *
 * APPROACH:
 * - Initialize window and target FPS
 * - Create Button objects for start/exit/restart
 * - Create Game object which loads audio and textures
 * - Run rendering loop until window close or exit button pressed
 * - Handle three UI states: game over screen, main menu (not running), and active game
 * - Clean up via destructors and CloseWindow
 */
int main()
{
    // Initialize a raylib window sized to fit the grid plus offsets
    InitWindow(2 * offset + cellsize * cellcount, 2 * offset + cellsize * cellcount, "Snake's world");
    SetTargetFPS(60); // cap framerate to 60 frames per second

    {
        // instantiate UI buttons; Button takes path, position and scale
        Button startButton{"graphics/start_button.png", {350, 300}, 0.65};
        Button exitButton{"graphics/exit_button.png", {350, 450}, 0.65};
        Button restartButton{"graphics/restart.png", {350, 500}, 1.5};

        bool exit = false; // control flag to break out of main loop when true
        Game game = Game(); // create and initialize game (loads sounds & fruits)

        // main loop: keep running while window is open and exit flag is false
        while (!WindowShouldClose() && exit == false)
        {
            BeginDrawing(); // start drawing frame
            ClearBackground(green); // clear with background color

            // allow Enter key to start the game when not already running
            if (IsKeyPressed(KEY_ENTER) && game.running == false)
            {
                game.running = true; // start/resume simulation
                game.game_over = false; // ensure game over flag cleared
            }

            if (game.game_over == true)
            {
                // render game over screen
                ClearBackground(green);
                DrawText("Game Over!", 220, 150, 90, darkGreen); // large headline

                Vector2 mousePosition = GetMousePosition(); // current mouse coords
                bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT); // check click

                // show last score and high score
                DrawText(TextFormat("Score: %i", temp_score), 350, 300, 60, darkGreen);
                DrawText(TextFormat("High Score: %i", high_score), 280, 400, 60, darkGreen);

                restartButton.Draw(); // draw restart button
                if (restartButton.isPressed(mousePosition, mousePressed))
                {
                    // if pressed, resume the game by clearing game_over and starting running
                    game.game_over = false;
                    game.running = true;
                }
            }
            else if ((!game.running) && (game.game_over == false))
            {
                // main menu state (not running and not game over)
                DrawText("Snake's World", 180, 150, 80, darkGreen);
                Vector2 mousePosition = GetMousePosition();
                bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                startButton.Draw(); // draw start button
                exitButton.Draw(); // draw exit button
                if (exitButton.isPressed(mousePosition, mousePressed))
                {
                    exit = true; // signal to exit outer loop
                }
                else if (startButton.isPressed(mousePosition, mousePressed))
                {
                    game.running = true; // start the game when start button pressed
                }
            }
            else
            {
                // active gameplay state
                game.Draw(); // draw snake and fruits

                // title and border for the playing grid
                DrawText("Snake's World", offset - 5, 20, 40, darkGreen);
                DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellsize * cellcount + 10, (float)cellsize * cellcount + 10}, 5, darkGreen);

                // display score and high score below the grid
                DrawText(TextFormat("Score: %i", game.score), offset - 10, offset + cellsize * cellcount + 10, 40, darkGreen);
                DrawText(TextFormat("High Score: %i", high_score), cellcount * cellsize - 185, offset + cellsize * cellcount + 10, 40, darkGreen);

                // trigger game update at intervals determined by game.speed
                if (EventTriggered(game.speed))
                {
                    game.Update(); // advance snake and perform collision checks
                }

                // input handling for direction changes: prevent reversal by checking current direction
                if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && game.snake.direction.y != 1)
                {
                    game.snake.direction = {0, -1}; // move up
                    WaitTime(0.15); // small debounce to avoid accidental multiple direction changes
                    game.running = true; // ensure simulation is active once a direction is pressed
                }
                if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && game.snake.direction.y != -1)
                {
                    game.snake.direction = {0, 1}; // move down
                    WaitTime(0.15);
                    game.running = true;
                }
                if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && game.snake.direction.x != 1)
                {
                    game.snake.direction = {-1, 0}; // move left
                    WaitTime(0.15);
                    game.running = true;
                }
                if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && game.snake.direction.x != -1)
                {
                    game.snake.direction = {1, 0}; // move right
                    WaitTime(0.15);
                    game.running = true;
                }
            }

            EndDrawing(); // finish drawing frame
        }
    }

    CloseWindow(); // close the raylib window and free resources
    return 0; // normal exit
}
