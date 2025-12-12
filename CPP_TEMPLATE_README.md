# C++ Project Quality Template

A complete, ready-to-use C++ project structure with automatic code formatting, linting, and static analysis.

## Quick Start

```bash
# 1. Clone or copy this template
cp -r cpp-quality-template my-new-project
cd my-new-project

# 2. Install dependencies (one-time)
sudo apt-get install clang-format cppcheck clang-tidy

# 3. Start coding
# Files auto-format on save in VS Code

# 4. Before committing
./format.sh
pio run
```

## What's Included

### Configuration Files
- **`.clang-format`** - Code formatting rules (Google style)
- **`.clang-tidy`** - Linting configuration
- **`.vscode/settings.json`** - VS Code auto-format settings

### Scripts
- **`format.sh`** - Run formatting + all linting checks

### Documentation
- **`CODE_QUALITY_GUIDE.md`** - Complete setup guide

## Directory Structure

```
project-root/
├── .clang-format              # Formatting rules
├── .clang-tidy               # Linting rules
├── .vscode/
│   └── settings.json         # VS Code settings
├── format.sh                 # Quality pipeline script
├── src/
│   └── main.cpp              # Your source files
├── include/
│   └── config.h              # Your headers
├── platformio.ini            # PlatformIO config
└── CODE_QUALITY_GUIDE.md     # This guide
```

## Quick Commands

```bash
# Format all code
./format.sh

# Format one file
clang-format -i src/main.cpp

# Check for issues
cppcheck --enable=all src/ include/

# Compile with PlatformIO
pio run

# Clean build
pio run --target clean
```

## How the Tools Work Together

1. **You write code** in VS Code
2. **VS Code auto-formats** on save (clang-format)
3. **You run** `./format.sh` before commit
4. **format.sh** runs:
   - clang-format (reformats all code)
   - cppcheck (finds bugs)
   - clang-tidy (modernizes code)
4. **Script generates** cppcheck_report.txt
5. **You compile** with `pio run`
6. **Commit** clean, professional code

## Customization

### Modify Code Style
Edit `.clang-format`:
- Change `IndentWidth` for different indentation
- Change `ColumnLimit` for line length
- See: https://clang.llvm.org/docs/ClangFormatStyleOptions/

### Disable Certain Checks
Edit `.clang-tidy`:
```yaml
Checks: >
  clang-analyzer-*,
  readability-*,
  -readability-magic-numbers  # Disable specific check
```

### Change Formatting Rules
Edit `.vscode/settings.json`:
```json
{
  "[cpp]": {
    "editor.formatOnSave": true,
    "editor.tabSize": 4  // Change tab size
  }
}
```

## For Different Project Types

### Arduino/PlatformIO Projects
- Keep `platformio.ini`
- Use as-is, add your code

### CMake Projects
- Remove `platformio.ini`
- Add `CMakeLists.txt`
- Scripts work the same way

### Make Projects
- Remove `platformio.ini`
- Add `Makefile`
- Scripts work the same way

## Dependencies

One-time install:
```bash
sudo apt-get update
sudo apt-get install clang-format cppcheck clang-tidy
```

Verify installation:
```bash
clang-format --version
cppcheck --version
clang-tidy --version
```

## VS Code Extensions

Install once:
```
xaver.clang-format  (ID: xaver.clang-format)
```

Or search in Extensions for "Clang-Format"

## Example Workflow

```bash
# Start project
mkdir my-project
cp -r cpp-quality-template/* my-project/
cd my-project

# Code as normal (auto-formats on save)
code .

# Before committing
./format.sh      # Check everything
pio run          # Compile
git add .
git commit -m "Add feature X"

# Done! Professional-grade code
```

## Troubleshooting

### format.sh permission denied
```bash
chmod +x format.sh
```

### clang-format not found
```bash
sudo apt-get install clang-format
```

### VS Code not auto-formatting
1. Install `xaver.clang-format` extension
2. Check `.vscode/settings.json` exists
3. Reload VS Code (Ctrl+Shift+P → Reload Window)

### cppcheck takes too long
Edit `format.sh` and reduce `--enable=all` to `--enable=style,performance`

## Next Steps

1. Copy entire template to new project
2. Customize `.clang-format` if needed
3. Update `platformio.ini` for your board
4. Start coding!

## Resources

- [clang-format docs](https://clang.llvm.org/docs/ClangFormat/)
- [cppcheck docs](http://cppcheck.sourceforge.net/)
- [clang-tidy docs](https://clang.llvm.org/extra/clang-tidy/)
- [PlatformIO docs](https://docs.platformio.org/)
