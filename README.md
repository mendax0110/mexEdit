# mexEdit - A Lightweight Terminal Text Editor

## Features

### Core Editing
- Syntax highlighting for c/cpp/python/shell/bash
- undo/redo functionality (Ctrl+Z/Ctrl+Y)
- Search/replace with regular expression support
- Command mode for advanced operations (ESC + :)
- Line number toggle (F4)

### File Management
- Integrated file explorer sidebar
- Save/open files with hotkeys (F2/F3)
- "Save As" functionality (F6)
- New file creation (F5)

## Installation

### Requirements
- C++17 compiler (GCC, Clang, etc.)
- CMake (version 3.10+)
- ncurses library

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/mendax0110/mexEdit.git

# Configure and build
cd mexEdit
mkdir build && cd build
cmake ..
cmake --build .

# Run the editor
./mexEdit [filename]