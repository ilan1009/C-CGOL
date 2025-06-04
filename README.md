# CCGOL – Optimized Conway's Game of Life in C

**CCGOL** is a high-performance simulation of Conway's Game of Life written in C, utilizing OpenGL for rendering with shaders and efficient data structures for large-scale pattern evolution. It supports loading RLE pattern files.

## Requirements

* **Compatible C compiler** (tested with gcc on linux, minGW on windows)
* **OpenGL**

## Installation

### Linux/macOS

```bash
make
```

### Windows

Use MinGW or a compatible compiler with:

```bash
make -f Makefile_win
```

## Usage

Navigate to build directory and run 
```bash
./CCGOL <grid size> <screen size>
```

* `<grid size>`: Number of simulation cells per axis (e.g. 256 for a 256x256 grid)
* `<screen size>`: Size of the application window in pixels (e.g. 1024)

### Example

```bash
./CCGOL 512 1080
```

### On Windows

```bash
CCGOL.exe 512 1080
```

## Controls

- **Arrow Up/Down**: Increase/Decrease simulation speed.
- **Space**: Pause/Resume the simulation.
- **Hold Tab**: Fast forward the simulation.
- **L**: Load RLE pattern files.
- **R**: Reset the simulation.

## RLE Pattern Files

Sample RLE files are located in the `rles/` directory. Patterns can be loaded during runtime.

## File Structure

```
build/             # Compiled objects and shaders
include/           # Header files for GLAD, GLFW, tinyfiledialogs, uthash.
libs/              # External C libraries (GLAD, tinyfiledialogs)
rles/              # Collection of RLE pattern files
src/               # Source code (game logic, rendering, window, etc.)
Makefile           # Linux/macOS build script
Makefile_win       # Windows build script
```