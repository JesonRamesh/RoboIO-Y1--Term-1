# RoboIO - Terminal-Based Robot Rescue Game

A thrilling console-based game where you control a rescue robot navigating through dangerous minefields to save trapped civilians. Built using C and the ncurses library for terminal graphics.

## 🎮 Game Overview

In RoboIO, you play as an elite rescue robot deployed to save people trapped in minefields. Navigate through increasingly challenging levels while avoiding deadly mines and collecting civilians to earn points.

### Story
The game begins with a dramatic conversation between a Commander and Soldier, where they realize their people are trapped in a minefield. With no one brave enough to enter, they call upon you - the most notorious rescue robot the world has ever seen!

## 🎯 Gameplay Features

- **Dynamic Minefield Navigation**: Avoid mines while rescuing civilians
- **AI-Powered Movement**: The robot automatically moves toward targets when not manually controlled
- **Progressive Difficulty**: Each level increases mine count and robot speed
- **Leaderboard System**: Track high scores across gaming sessions
- **Colorful Terminal Graphics**: Enhanced visual experience with color-coded elements
- **Real-time Collision Detection**: Immediate feedback on mine hits and wall collisions

## 🕹️ Controls

- **Arrow Keys**: Control robot direction
  - ↑ (Up): Move North
  - ↓ (Down): Move South  
  - ← (Left): Move West
  - → (Right): Move East
- **Q**: Quit game
- **Any Key**: Continue through story/level screens

## 🎮 Game Mechanics

### Objective
- Rescue civilians (yellow 'o') while avoiding mines (red '.')
- Each rescued civilian increases your score by 1
- Survive as long as possible with your 3 lives

### Difficulty Progression
- **Level Up**: Every 5 people rescued advances to the next level
- **Increased Challenge**: More mines spawn each level (max 50 mines)
- **Faster Movement**: Robot speed increases with each level
- **Dynamic Mine Placement**: Mine positions change every 2 rescues

### Collision System
- **Mine Hit**: Lose 1 life, robot resets to center
- **Wall Collision**: Lose 1 life, robot resets to center
- **Game Over**: When all 3 lives are lost

### AI Behavior
When not manually controlled, the robot uses intelligent pathfinding to:
- Move toward the nearest civilian
- Avoid mines and walls
- Calculate optimal routes using distance algorithms

## 🛠️ Technical Requirements

### Dependencies
- C compiler (GCC recommended)
- ncurses library
- Standard C libraries (stdlib, time, string, unistd, math)

### Installation

1. **Install ncurses** (Ubuntu/Debian):
   ```bash
   sudo apt-get install libncurses5-dev libncursesw5-dev
   ```

2. **Compile the game**:
   ```bash
   gcc -o roboio roboio.c -lncurses -lm
   ```

3. **Run the game**:
   ```bash
   ./roboio
   ```

### System Requirements
- Terminal with color support
- Minimum terminal size: 100x20 characters
- Linux/Unix environment (tested on Ubuntu)

## 🎨 Visual Elements

- **Robot**: Green 'o' with directional indicators (^, v, <, >)
- **Civilians**: Yellow 'o' 
- **Mines**: Red '.'
- **Game Board**: Bordered rectangular playing field
- **UI Elements**: Player name, lives, score, and level display

## 📊 Scoring System

- **+1 Point**: Each civilian rescued
- **Level Progression**: Every 5 points advances one level
- **High Score Tracking**: Persistent leaderboard saved to file
- **Maximum Score**: Theoretical maximum of 48+ points (when mine limit reached)

## 🏆 Leaderboard

The game maintains a persistent leaderboard system:
- Scores saved to `leaderboard.txt`
- Top 10 players displayed on game over
- Automatic sorting by highest score
- Player names and scores tracked across sessions

## 🔧 Code Structure

### Key Components
- **Game Loop**: Real-time gameplay with configurable delay
- **Collision Detection**: Efficient mine and wall collision checking
- **AI Movement**: Intelligent pathfinding algorithm
- **Dynamic Memory Management**: Allocated memory for mine positions
- **File I/O**: Persistent leaderboard storage
- **Terminal Graphics**: ncurses-based rendering system

### Main Functions
- `main()`: Game initialization and primary game loop
- `move_robot_ai()`: AI pathfinding and movement logic
- `check_collision()`: Collision detection system
- `random_coordinates_*()`: Dynamic positioning algorithms
- `save_score()` / `show_leaderboard()`: Score management

## 🚀 Future Enhancements

Potential improvements for the game:
- Power-ups and special abilities
- Multiple robot types with different capabilities
- Multiplayer support
- Sound effects and enhanced graphics
- Additional obstacle types
- Save/load game functionality
- More sophisticated AI algorithms

## 🐛 Known Issues

- Game requires specific terminal size for optimal display
- Color support may vary across different terminal emulators
- File permissions needed for leaderboard functionality

## 📝 Development Notes

This project was developed as a first-year computer science assignment, demonstrating:
- C programming fundamentals
- Memory management with malloc/free
- File I/O operations
- Terminal-based user interfaces
- Game loop architecture
- Algorithm implementation (pathfinding, collision detection)

## 🎓 Educational Value

RoboIO serves as an excellent example of:
- **Data Structures**: Structs for game entities, arrays for mine positions
- **Algorithms**: AI pathfinding, collision detection, sorting algorithms
- **System Programming**: Terminal control, real-time input handling
- **Software Engineering**: Modular code design, function organization

## 📄 License

This project was created for educational purposes. Feel free to use and modify for learning and non-commercial purposes.

---

**Enjoy saving lives in the digital minefield! 🤖💥**
