# Cellular Automata Simulator

An interactive, infinite-canvas Cellular Automata engine that lets you design custom rulesets and observe emerging complex patterns in real time.

---

## 🌟 Features

- **Custom Rulesets:** Easily define and experiment with different cellular automata rules (e.g., _Conway's Game of Life_ `B3/S23`, _HighLife_ `B36/S23`, and custom variations).
- **Infinite Canvas:** Drag, pan, and zoom freely across an unbounded world grid.
- **Flexible Simulation Control:** Toggle auto-simulation, adjust execution speeds dynamically, or advance frame-by-frame manually.
- **Quick Utilities:** Instantly seed the grid randomly or clear the canvas with simple hotkeys.

---

## ⌨️ Controls & Keybinds

### Navigation & Painting

| Action            | Input                                                            |
| :---------------- | :--------------------------------------------------------------- |
| **Toggle Cell**   | <kbd>Left Click</kbd>                                            |
| **Drag Camera**   | <kbd>Right Click</kbd> + Drag                                    |
| **Zoom In / Out** | <kbd>Mouse Wheel</kbd>                                           |
| **Move Camera**   | <kbd>↑</kbd> <kbd>↓</kbd> <kbd>←</kbd> <kbd>→</kbd> (Arrow Keys) |

### Simulation Controls

| Action                | Input            |
| :-------------------- | :--------------- |
| **Start / Pause**     | <kbd>Space</kbd> |
| **Step Single Frame** | <kbd>X</kbd>     |
| **Increase Speed**    | <kbd>L</kbd>     |
| **Decrease Speed**    | <kbd>H</kbd>     |
| **Double Speed**      | <kbd>J</kbd>     |
| **Halve Speed**       | <kbd>K</kbd>     |

### Utilities

| Action                 | Input          |
| :--------------------- | :------------- |
| **Randomize Canvas**   | <kbd>R</kbd>   |
| **Clear Canvas**       | <kbd>C</kbd>   |
| **Toggle Debug Stats** | <kbd>F3</kbd>  |
| **Toggle Fullscreen**  | <kbd>F11</kbd> |
| **Exit**               | <kbd>Esc</kbd> |

### Blueprints

| Action                       | Input            |
| :--------------------------- | :--------------- |
| **Choose Blueprint**         | <kbd>B</kbd>     |
| **Paste Blueprint At Mouse** | <kbd>Enter</kbd> |

---

## 🛠️ Requirements & Building

### Prerequisites

- **C++ Compiler:** Supporting **C++20** standard (GCC 10+, Clang 10+, or MSVC 2019+).
- **Build System:** CMake

### Quick Start

```bash
# Clone the repository
git clone https://github.com/nixuh3/Game-of-Life.git
cd game-of-life

# Build project
cmake -B build
cmake --build build

# Run the executable
.\build\game-of-life # Path to executable
```
