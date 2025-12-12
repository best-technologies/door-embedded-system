# Quick Reference: Using the Setup Script

## For Your Current Project (already done)

You already have everything set up! Just use it:

```bash
# Before each commit
./format.sh

# Compile
pio run
```

---

## For New C++ Projects

### Method 1: Copy the Template (Easiest)

```bash
# Clone current project as template
cp -r best_tech_door my-new-project
cd my-new-project

# Remove old project files
rm -rf src/door_system.cpp src/controller_esp32.cpp src/main.cpp
rm -rf include/auth_config.h include/wifi_manager.h
rm .git/

# Now you have clean template with all quality tools!
```

### Method 2: Run Setup Script in New Project (Recommended)

```bash
# Create new project
mkdir my-new-cpp-project
cd my-new-cpp-project

# Initialize as PlatformIO project
pio project init --board esp32doit-devkit-v1  # or your board

# OR initialize with git
git init
git clone <your-repo>
cd <repo-folder>

# Run setup script
bash /path/to/setup-cpp-quality.sh

# Or if you have it in your repo
bash ../best_tech_door/setup-cpp-quality.sh

# Done! All quality tools installed
```

---

## What the Setup Script Does (Automatic)

1. ✓ Checks if clang-format, cppcheck, clang-tidy are installed
2. ✓ Installs them if missing (with sudo)
3. ✓ Creates `.clang-format` (formatting rules)
4. ✓ Creates `.clang-tidy` (linting rules)
5. ✓ Creates `.vscode/settings.json` (VS Code auto-format)
6. ✓ Creates `format.sh` (quality pipeline)

**Total time:** ~2 minutes

---

## Step-by-Step for Complete Beginner

### First Time Only (One-time setup):

```bash
# 1. Go to your project
cd my-cpp-project

# 2. Copy setup script (from best_tech_door folder)
cp ~/Documents/challenge/Btech_door/best_tech_door/setup-cpp-quality.sh .

# 3. Run it
bash setup-cpp-quality.sh

# 4. Install VS Code extension
# - Open VS Code
# - Ctrl+Shift+X (Extensions)
# - Search "xaver.clang-format"
# - Click Install

# 5. Reload VS Code
# - Ctrl+Shift+P
# - Type "Reload Window"
# - Press Enter
```

### Every Time You Code:

```bash
# Just code normally
# Files auto-format on save in VS Code

# Before committing
./format.sh
git add .
git commit -m "Your message"
```

---

## Keep the Setup Script Somewhere Safe

### Option 1: In GitHub (Best)

```bash
# Add to your best_tech_door repo
git add setup-cpp-quality.sh CPP_TEMPLATE_README.md
git commit -m "Add C++ quality setup template"
git push

# Then for any new project:
# 1. Create new folder
# 2. Download setup-cpp-quality.sh from GitHub
# 3. Run it
```

### Option 2: In a Template Folder

```bash
# Create a templates folder
mkdir -p ~/templates/cpp-quality
cp setup-cpp-quality.sh ~/templates/cpp-quality/
cp .clang-format ~/templates/cpp-quality/
cp .clang-tidy ~/templates/cpp-quality/
cp CPP_TEMPLATE_README.md ~/templates/cpp-quality/

# For new projects:
cp ~/templates/cpp-quality/setup-cpp-quality.sh /path/to/new-project/
cd /path/to/new-project/
bash setup-cpp-quality.sh
```

### Option 3: Create an Alias (Super Fast)

```bash
# Add to ~/.zshrc
alias setup-cpp-quality='bash ~/Documents/challenge/Btech_door/best_tech_door/setup-cpp-quality.sh'

# Reload shell
source ~/.zshrc

# Now any project:
cd my-new-project
setup-cpp-quality
```

---

## Common Scenarios

### Scenario 1: New PlatformIO Project

```bash
pio project init --board esp32doit-devkit-v1
cd .pio/project_init/
bash ~/Documents/challenge/Btech_door/best_tech_door/setup-cpp-quality.sh
```

### Scenario 2: Existing Project (No Quality Tools)

```bash
cd /path/to/existing/project
bash ~/Documents/challenge/Btech_door/best_tech_door/setup-cpp-quality.sh
# Done! All quality tools installed
```

### Scenario 3: CMake Project

```bash
# Setup works the same
cd /path/to/cmake/project
bash ~/Documents/challenge/Btech_door/best_tech_door/setup-cpp-quality.sh
# (platformio.ini not needed)
```

### Scenario 4: Arduino IDE Project

```bash
# Create proper structure first
mkdir -p my-arduino-project/src my-arduino-project/include
cd my-arduino-project
bash ~/Documents/challenge/Btech_door/best_tech_door/setup-cpp-quality.sh
```

---

## Verify Setup Works

```bash
# 1. Check files exist
ls -la .clang-format .clang-tidy format.sh .vscode/settings.json

# 2. Test formatting
clang-format --version
cppcheck --version
clang-tidy --version

# 3. Try it
./format.sh

# 4. Check VS Code integration
# - Open a .cpp file
# - Make bad formatting (weird spacing)
# - Save (Ctrl+S)
# - Should auto-format instantly
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `Permission denied: ./format.sh` | `chmod +x format.sh` |
| `command not found: clang-format` | `sudo apt-get install clang-format` |
| `VS Code not auto-formatting` | 1. Install extension, 2. Check settings.json, 3. Reload |
| `setup script not executable` | `bash setup-cpp-quality.sh` (don't need `./`) |

---

## Summary

**You don't need me anymore!** 

Just:
1. Copy `setup-cpp-quality.sh` to new projects
2. Run it once: `bash setup-cpp-quality.sh`
3. Code normally (auto-format on save)
4. Before commit: `./format.sh`

That's it! Professional C++ code every time. 🚀
