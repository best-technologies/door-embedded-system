# C++ Code Quality Setup Checklist

## ✅ Installation & Configuration

- [ ] Install tools: `sudo apt-get install clang-format cppcheck clang-tidy`
- [ ] `.clang-format` file created ✓
- [ ] `.clang-tidy` file created ✓
- [ ] `.vscode/settings.json` configured ✓
- [ ] VS Code extension installed: `xaver.clang-format`
- [ ] `format.sh` script created and executable ✓

## 🔄 VS Code Automation (Happens Automatically)

- Auto-format on save enabled
- Auto-format on paste enabled
- C/C++ code analysis enabled

## 📋 Manual Commands

```bash
# Format all code
./format.sh

# Format single file
clang-format -i src/door_system.cpp

# Check specific file
cppcheck src/door_system.cpp

# Check entire project
cppcheck --enable=all --include-path=include/ src/ include/

# Run linting
clang-tidy src/door_system.cpp -- -I./include/
```

## 🚀 Recommended Workflow

1. **While Coding**: 
   - VS Code auto-formats on save
   - Real-time code analysis shows warnings

2. **Before Commit**:
   ```bash
   ./format.sh          # Run complete pipeline
   pio run               # Compile to verify
   git add .
   git commit -m "..."
   ```

3. **In CI/CD** (GitHub Actions):
   - Run `./format.sh` automatically
   - Check for formatting violations
   - Block merge if issues found

## 📊 What Gets Checked

- Code formatting (spaces, indentation, braces)
- Unused variables and functions
- Memory leaks and null pointer issues
- Logic errors and potential bugs
- Modern C++ best practices
- Performance improvements
- Code complexity warnings

## 🎯 Expected Output

```
═════════════════════════════════════════════════════
   C++ Code Quality Pipeline
═════════════════════════════════════════════════════

📦 Checking dependencies...
✓ clang-format installed
✓ cppcheck installed
✓ clang-tidy installed

📝 Step 1: Formatting code with clang-format...
✓ Formatting complete

🔎 Step 2: Running cppcheck (static analysis)...
✓ Cppcheck passed

🔍 Step 3: Running clang-tidy (advanced linting)...
✓ Clang-tidy analysis complete

═════════════════════════════════════════════════════
✅ Pipeline complete!
═════════════════════════════════════════════════════
```
