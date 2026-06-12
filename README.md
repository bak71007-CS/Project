# Asteroid Shooter Game

**Course:** Programming Fundamentals (PF) Lab - CL1002  
**Language:** C  
**Target Platform:** Windows Console  

## Description
Asteroid Shooter is a survival-style arcade game developed in C. The player controls a ship at the bottom of the screen and must destroy falling asteroids to earn points. The game features a progression system where the difficulty increases as the player’s score rises, making asteroids move faster and appear more frequently.

## Features
* **Progressive Difficulty:** The game uses mathematical formulas to adjust asteroid speed and spawn rates based on the current score.
* **Efficient Logic:** Uses a stack-based system to manage active bullets, ensuring the game runs smoothly even with multiple objects on screen.
* **Smooth Rendering:** Utilizes the Windows API to refresh the screen, preventing the flickering often found in basic console applications.
* **Collision System:** A dedicated logic engine handles hit detection between bullets and asteroids, as well as player-asteroid impacts.

## Prerequisites
This game is designed for Windows environments as it utilizes the following libraries:
* `<windows.h>`: For console cursor control and frame timing.
* `<conio.h>`: For non-blocking keyboard input.

## Installation and Execution
1. Ensure you have a C compiler installed, such as **MinGW** or **GCC**.
2. Download the source code file (e.g., `main.c`).
3. Open the command prompt and navigate to the project directory.
4. Compile the program using the following command:
   ```bash
   gcc main.c -o AsteroidShooter
