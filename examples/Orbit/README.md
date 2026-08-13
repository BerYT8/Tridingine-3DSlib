# Orbit

**Orbit** is a small 2D game made with **Tridingine-3DSlib**.

You play as a circle while enemy squares appear around the edges of the screen and chase you.

Collect the orbs to increase your score. When an orb is collected, the enemies currently chasing you disappear.

If an enemy touches you, the game ends.

## 🎮 Gameplay

* 🟢 Control the player circle.
* 🔴 Avoid the enemy squares.
* 🔵 Collect orbs to increase your score.
* 💥 Collecting an orb removes the current enemies.
* 🏆 Try to beat your high score.
* 💀 Survive for as long as possible.

---

## 🛠️ What it demonstrates

Orbit is included as an example of how different parts of Tridingine-3DSlib can be combined to create a complete game.

It currently demonstrates:

* 2D rendering
* Geometric primitives
* Text rendering
* Input
* Delta time
* Random number generation
* Colors
* Game state
* Collision detection
* Score handling
* High score / save system
* Basic game logic

The game intentionally uses simple geometric graphics so that the engine's functionality remains easy to see.

---

## 🖥️ Platforms

Orbit is designed to run on:

* **Windows**
* **Linux**
* **macOS**
* **Nintendo 3DS**

The same game code is used across the supported platforms whenever possible.

---

## 🚀 Building

Orbit is included with the Tridingine-3DSlib examples.

First, follow the engine installation instructions:

**[Installation](../01-Installation.md)**

Then build the generated project using the appropriate platform.

### PC

```bash
./build.sh
```

On Windows:

```bat
build.bat
```

### Nintendo 3DS

On Windows:

```bat
build.bat 3ds
```

On Linux:

```bash
./build.sh build_3ds
```

For the complete build instructions, see the main documentation.

---

## 📸 Screenshots

![Orbit gameplay](game_shots/game_view.gif)

---

## 📜 About

Orbit is a demonstration project for **Tridingine-3DSlib** and is primarily intended to show how a small complete game can be built using the engine.

For more information about the engine:

**[Tridingine-3DSlib](../../README.md)**
