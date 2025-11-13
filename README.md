<<<<<<< HEAD
# ancient-warriors-game
=======
# 🥋 Ancient Warriors - Collectibles Game

A computer graphics game featuring ancient East Asian warriors (Ninjas/Samurais) where you collect items from different platforms while battling against time!

## Quick Start

### Run the Game (Easy Way)
```bash
./run.sh
```

That's it! The script will automatically build and run the game.

---

## 🎮 How to Play

### Objective
Collect all **12 collectibles** from 4 platforms within **120 seconds** to win!

### Gameplay Flow

1. **Start Game**
   - You spawn in the center of the arena as a warrior character
   - There are 4 platforms around you (in different colors)
   - Each platform has 3 collectible items (golden orbs with rings)
   - Timer starts at 120 seconds

2. **Collect Items**
   - **Move** your character using `W/A/S/D` or **Arrow Keys**
   - Walk toward the floating **golden collectibles** (they rotate continuously)
   - Get close to a collectible and it will automatically be collected
   - The HUD shows your progress: "Collected: X/12"

3. **After Collecting from a Platform**
   - Once you collect all 3 items from a platform, it becomes "complete"
   - The HUD will show a ✓ checkmark for that platform
   - You can now trigger **animations** on that platform using `Z/X/C/V`

4. **Win Condition**
   - Collect all 12 items from all 4 platforms
   - Win screen appears with congratulations message!

---

## ⌨️ Controls

### Movement
- **W** - Move forward
- **A** - Move left
- **S** - Move backward
- **D** - Move right
- **Arrow Keys** - Alternative movement

### Camera Views
- **1** - Top-down view (bird's eye)
- **2** - Side view
- **3** - Front view
- **0** - Free camera mode (default)

### Free Camera (Mode 0)
- **Click + Drag** - Rotate camera around character
- **Scroll Wheel** - Zoom in/out

### Animations (after collecting all items from each platform)
- **Z** - Toggle animation for Platform 1 (Lantern - Rotation)
- **X** - Toggle animation for Platform 2 (Pagoda - Scaling)
- **C** - Toggle animation for Platform 3 (Statue - Translation/Bouncing)
- **V** - Toggle animation for Platform 4 (Weapon Rack - Color Change)

### Game Controls
- **R** - Restart game
- **ESC** - Exit game

---

## 🏛️ Platforms Overview

| Platform | Location | Object | Animation | Items |
|----------|----------|--------|-----------|-------|
| **Platform 1** | Bottom-Left (Red) | Lantern | Rotating 360° | 3 |
| **Platform 2** | Bottom-Right (Green) | Pagoda Tower | Scaling up/down | 3 |
| **Platform 3** | Top-Left (Blue) | Statue | Bouncing up/down | 3 |
| **Platform 4** | Top-Right (Yellow) | Weapon Rack | Color Shifting | 3 |

---

## 📊 HUD (On-Screen Display)

The game displays helpful information on your screen:

- **Time: MM:SS** - Remaining time (top-left)
- **Collected: X/12** - Items collected (top-left)
- **Platforms:** - Status of each platform (top-left)
  - P1: ✓ [ON] = Collected & Animation active
  - P1: ✓ [OFF] = Collected & Animation inactive
  - P1: ✗ [OFF] = Not fully collected yet
- **Controls** - Displayed at bottom of screen

---

## 🎯 Tips & Tricks

1. **Plan your route** - Visit platforms in an efficient order to save time
2. **Watch the timer** - You have 120 seconds (2 minutes)
3. **Use different camera views** - Switch views to find hard-to-see collectibles
4. **Free camera** - Use mouse control (mode 0) to get better angles
5. **Animations are rewards** - Trigger them after collecting to see special effects!

---

## 🔧 Building Manually

If you want to build from scratch:

```bash
g++ -std=c++17 src/P01_1234.cpp -o src/P01_1234 -framework OpenGL -framework GLUT -Wno-deprecated
```

Then run:
```bash
./src/P01_1234
```

---

## 📋 Requirements

- macOS (uses OpenGL and GLUT frameworks)
- g++ compiler with C++17 support
- OpenGL and GLUT libraries (pre-installed on macOS)

---

## 🎨 Game Features

- ✅ 3D Graphics using OpenGL
- ✅ 4 unique decorated platforms with collectibles
- ✅ Smooth character movement and collision detection
- ✅ Multiple camera view modes
- ✅ Platform animations (rotation, scaling, translation, color change)
- ✅ Real-time timer
- ✅ Win/Game Over screens
- ✅ Smooth lighting and shading
- ✅ Interactive UI and controls

---

Enjoy the game! 🥋✨
>>>>>>> d9ab04e (Game)
