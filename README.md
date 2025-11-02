# mexEdit - A Lightweight Terminal Text Editor

A simple, fast terminal-based text editor built with C++ and ncurses.

## Features

### Core Editing
- **Syntax highlighting** for keywords, strings, comments, numbers, and operators
- **Undo/redo** functionality (Ctrl+Z/Ctrl+Y) 
- **Text selection** with copy/paste (Ctrl+C/Ctrl+V/Ctrl+X)
- **Select all** (Ctrl+A)
- **Line numbers** with toggle (F4)
- **Command mode** for advanced operations (ESC)

### File Management
- **File explorer sidebar** with directory navigation (F12 to toggle)
- **Open files** - Press Enter or F3 in file explorer
- **Save files** (F2 or Ctrl+S)
- **New file creation** (Ctrl+N)
- **Navigate directories** in file explorer with arrow keys

### User Interface
- **Clean, modern interface** with proper borders and colors
- **Status bar** showing file info, cursor position, and help
- **Responsive layout** that adapts to terminal size
- **Memory-safe** with built-in debugging support

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **F1** | Help |
| **F2** | Save file |
| **F3** | Open file (in explorer) |
| **F4** | Toggle line numbers |
| **F7** | Quit |
| **F12** | Toggle file explorer |
| **Ctrl+C** | Copy selection |
| **Ctrl+V** | Paste |
| **Ctrl+X** | Cut selection |
| **Ctrl+Z** | Undo |
| **Ctrl+Y** | Redo |
| **Ctrl+A** | Select all |
| **Ctrl+S** | Save |
| **Ctrl+N** | New file |
| **Enter** | Open file (in explorer) or new line |
| **ESC** | Command mode |

## Installation

### Requirements
- **C++20** compatible compiler (GCC, Clang)
- **CMake** 3.16+
- **ncurses** library
- **pkg-config** (for dependency management)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/mendax0110/mexEdit.git
cd mexEdit

# Build with CMake
mkdir build && cd build
cmake ..
make

# Run the editor
./mexEdit [optional-filename]
```

### Linux Package Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential cmake libncurses5-dev libncursesw5-dev pkg-config
```

**Fedora/CentOS:**
```bash
sudo dnf install gcc-c++ cmake ncurses-devel pkgconfig
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake ncurses pkg-config
```

## Usage

### Basic Usage
```bash
# Open mexEdit with a new file
./mexEdit

# Open an existing file
./mexEdit myfile.cpp

# Open mexEdit and navigate to a file using the file explorer
./mexEdit
# Press F12 to open file explorer, use arrows to navigate, Enter to open
```

### File Explorer
- Use **arrow keys** to navigate files and directories
- Press **Enter** to open files or enter directories  
- Press **F12** to toggle the file explorer on/off
- **[DIR]** prefix indicates directories

### Command Mode
Press **ESC** to enter command mode, then type:
- `:help` - Show available commands
- `:open <filename>` - Open a file
- `:save [filename]` - Save current file
- `:new` - Create new file
- `:quit` - Quit editor
- `:toggle-explorer` - Toggle file explorer
- `:toggle-numbers` - Toggle line numbers

## Development

### Project Structure
```
mexEdit/
├── src/                 # Source files
│   ├── core/           # Core editor functionality
│   ├── features/       # Features like syntax highlighting
│   ├── ui/             # User interface (ncurses)
│   └── utils/          # Utility classes
├── include/            # Header files
├── build/              # Build directory (created by cmake)
└── CMakeLists.txt      # Build configuration
```

### Memory Safety
mexEdit includes built-in memory debugging features. Build with debug mode to enable:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## Author

Created by [mendax0110](https://github.com/mendax0110)