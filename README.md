# 🐍 Snake’s World  
*A C++ Retro Snake Game Built with Raylib*

---

## 🎮 Overview

**Snake’s World** is a modern re-implementation of the classic Snake game, built using **C++** and **Raylib**.  
The game features smooth controls, animated fruit textures, sound effects, UI buttons, and a clean game-state architecture.

This project was developed as part of an Object-Oriented Programming Systems (OOPS) practical assignment.

---

## 👥 Team Contributions

| Member | Contribution |
|--------|--------------|
| **Piyush Khanna** | Game Logic + Game Design |
| **Tanubhav Juneja** | Game Logic + Game Design |
| **Rashmi Shah** | Game Logic + Documentation |
| **Gunjal Manocha** | Documentation + QA |

---

## 🚀 Features

✔ Smooth snake movement  
✔ Grid-based 2D world  
✔ Multiple fruit textures (chosen randomly)  
✔ Increasing snake speed as score rises  
✔ Collision sounds (eat, wall hit)  
✔ Clean game UI with buttons: Start, Exit, Restart  
✔ Game states: **Menu → Running → Game Over**  
✔ High-score tracking  
✔ Memory-safe asset loading using static textures  

---

🧩 How the Game Works
🎮 Game States
Main Menu → Running → Game Over
                    ↘ Restart

🕹 Controls
| Key | Action |
|-----|--------|
| ⬆ / W | Move Up |
| ⬇ / S | Move Down |
| ⬅ / A | Move Left |
| ➡ / D | Move Right |
| ENTER | Start Game |

---

## 🧠 Core Components

### 🐍 `Snake` Class
Handles movement, growth, and rendering.  
Key Members: body (deque), direction (Vector2), addSegment flag.  
Key Methods: `Draw()`, `Update()`, `Reset()`.

---

### 🍎 `Food` Class
Handles fruit placement and textures.  
Static textures loaded once; each fruit has `position` and `textureIndex`.

---

### 🎮 `Game` Class
Central controller for gameplay, audio, score, and state transitions.  
Key Methods: Constructor/Destructor (assets), `CheckCollisionWithFood()`, `CheckCollisionWithEdges()`, `CheckCollisionsWithTail()`, `Update()`, `GameOver()`.

---

## 🎨 Rendering & UI

Raylib is used for drawing rectangles, textures, text, and handling audio/input.  
UI buttons are implemented via `button.hpp` and use mouse click detection.

---

## 🛠 Build & Run Instructions

### **Prerequisites**
Install **Raylib** for your platform and a C++ compiler (g++/clang).
Windows (MinGW), Linux, or macOS is supported.

Compile
```bash
g++ main.cpp -o snake
```
Run
```bash
./snake
```

---

## 📐 Gameplay Mechanics

### 🍎 Food Eating

Score increments

New fruit appears randomly

Snake grows

Speed increases (speed *= 0.98 until minimum limit)

---

### 💥 Collision Handling

Game Over occurs when:

Snake hits walls

Snake hits its own tail

---

## 📦 Global Configuration

Variable	Purpose
cellsize = 30	Size of each grid tile
cellcount = 25	Grid dimension (25×25)
offset = 75	Margin around game grid
speed = 0.2	Initial movement interval
high_score	Stored until program exit

---

## ✨ Possible Extensions

Add obstacles or maze mode

Add skin selector for snake

---

Collision systems

UI interaction with custom buttons

This project provides a strong foundation for expanding into more advanced 2D game development.
