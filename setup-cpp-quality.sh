#!/bin/bash
# Auto-setup script for C++ project quality tools
# Run this once in any C++ project

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}"
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║        C++ Project Quality Setup Wizard                      ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check if we're in a project directory
if [ ! -f "platformio.ini" ] && [ ! -f "CMakeLists.txt" ] && [ ! -f "Makefile" ]; then
    echo -e "${YELLOW}⚠ Warning: No platformio.ini, CMakeLists.txt, or Makefile found${NC}"
    echo "This script works best in a project directory"
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Step 1: Check dependencies
echo -e "\n${YELLOW}Step 1: Checking dependencies...${NC}"
deps_ok=true

for cmd in clang-format cppcheck clang-tidy; do
    if command -v $cmd &> /dev/null; then
        version=$($cmd --version 2>&1 | head -1)
        echo -e "${GREEN}✓${NC} $cmd installed ($version)"
    else
        echo -e "${RED}✗${NC} $cmd not found"
        deps_ok=false
    fi
done

if [ "$deps_ok" = false ]; then
    echo -e "\n${YELLOW}Installing missing tools...${NC}"
    sudo apt-get update
    sudo apt-get install -y clang-format cppcheck clang-tidy
    echo -e "${GREEN}✓ Installation complete${NC}"
fi

# Step 2: Create .clang-format
echo -e "\n${YELLOW}Step 2: Creating .clang-format...${NC}"
if [ -f ".clang-format" ]; then
    echo -e "${YELLOW}⚠ .clang-format already exists, skipping${NC}"
else
    cat > .clang-format << 'EOF'
---
Language: Cpp
BasedOnStyle: Google
IndentWidth: 2
UseTab: Never
ColumnLimit: 100
AlignAfterOpenBracket: AlignAfterOpenBracket
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
BraceWrapping:
  AfterClass: false
  AfterControlStatement: false
  AfterEnum: false
  AfterFunction: false
  AfterNamespace: false
  AfterStruct: false
  AfterUnion: false
  BeforeCatch: false
  BeforeElse: false
BreakBeforeBinaryOperators: NonAssignment
BreakConstructorInitializers: BeforeColon
Cpp11BracedListStyle: true
FixNamespaceComments: true
IncludeBlocks: Regroup
NamespaceIndentation: All
PointerAlignment: Left
ReflowComments: true
SortIncludes: true
SortUsingDeclarations: true
SpaceAfterCStyleCast: false
SpaceBeforeAssignmentOperators: true
SpaceBeforeParens: ControlStatements
Standard: c++17
EOF
    echo -e "${GREEN}✓ Created .clang-format${NC}"
fi

# Step 3: Create .clang-tidy
echo -e "\n${YELLOW}Step 3: Creating .clang-tidy...${NC}"
if [ -f ".clang-tidy" ]; then
    echo -e "${YELLOW}⚠ .clang-tidy already exists, skipping${NC}"
else
    cat > .clang-tidy << 'EOF'
---
Checks: >
  clang-analyzer-*,
  readability-*,
  modernize-*,
  performance-*,
  portability-*,
  -readability-magic-numbers,
  -readability-implicit-bool-conversion
WarningsAsErrors: '*'
HeaderFilterRegex: '.*'
EOF
    echo -e "${GREEN}✓ Created .clang-tidy${NC}"
fi

# Step 4: Create .vscode directory and settings
echo -e "\n${YELLOW}Step 4: Creating .vscode/settings.json...${NC}"
mkdir -p .vscode

if [ -f ".vscode/settings.json" ]; then
    echo -e "${YELLOW}⚠ .vscode/settings.json already exists, backing up${NC}"
    mv .vscode/settings.json .vscode/settings.json.backup
fi

cat > .vscode/settings.json << 'EOF'
{
  "C_Cpp.formatting": "clangFormat",
  "[cpp]": {
    "editor.defaultFormatter": "xaver.clang-format",
    "editor.formatOnSave": true,
    "editor.formatOnPaste": true,
    "editor.codeActionsOnSave": {
      "source.fixAll": true
    }
  },
  "[c]": {
    "editor.defaultFormatter": "xaver.clang-format",
    "editor.formatOnSave": true
  },
  "C_Cpp.codeAnalysis.enabled": true,
  "C_Cpp.codeAnalysis.runCodeAnalysis": "onSave"
}
EOF
echo -e "${GREEN}✓ Created .vscode/settings.json${NC}"

# Step 5: Create format.sh
echo -e "\n${YELLOW}Step 5: Creating format.sh...${NC}"
if [ -f "format.sh" ]; then
    echo -e "${YELLOW}⚠ format.sh already exists, backing up${NC}"
    mv format.sh format.sh.backup
fi

cat > format.sh << 'SCRIPT'
#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}═══════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}   C++ Code Quality Pipeline${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════${NC}"

# Check dependencies
echo -e "\n${YELLOW}📦 Checking dependencies...${NC}"
deps_missing=0
for cmd in clang-format cppcheck clang-tidy; do
    if ! command -v $cmd &> /dev/null; then
        echo -e "${RED}❌ $cmd not found${NC}"
        deps_missing=1
    else
        echo -e "${GREEN}✓ $cmd installed${NC}"
    fi
done

if [ $deps_missing -eq 1 ]; then
    echo -e "\n${RED}Install missing tools:${NC}"
    echo "  sudo apt-get install clang-format cppcheck clang-tidy"
    exit 1
fi

# Format code
echo -e "\n${YELLOW}📝 Formatting code with clang-format...${NC}"
if [ -d "src" ]; then
    find src -name "*.cpp" -o -name "*.h" 2>/dev/null | xargs clang-format -i 2>/dev/null || true
fi
if [ -d "include" ]; then
    find include -name "*.h" 2>/dev/null | xargs clang-format -i 2>/dev/null || true
fi
echo -e "${GREEN}✓ Formatting complete${NC}"

# Run cppcheck
echo -e "\n${YELLOW}🔎 Running cppcheck...${NC}"
if [ -d "src" ] && [ -d "include" ]; then
    cppcheck --enable=all --include-path=include/ \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        src/ include/ 2>&1 | tee cppcheck_report.txt || true
    echo -e "${GREEN}✓ Analysis complete (see cppcheck_report.txt)${NC}"
elif [ -d "src" ]; then
    cppcheck --enable=all src/ 2>&1 | tee cppcheck_report.txt || true
    echo -e "${GREEN}✓ Analysis complete${NC}"
fi

echo -e "\n${YELLOW}═══════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}✅ Quality checks complete!${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════${NC}"
SCRIPT

chmod +x format.sh
echo -e "${GREEN}✓ Created format.sh${NC}"

# Step 6: Summary
echo -e "\n${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║                    Setup Complete! ✅                         ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"

echo -e "\n${GREEN}What's been installed:${NC}"
echo "  ✓ .clang-format (code formatting rules)"
echo "  ✓ .clang-tidy (linting rules)"
echo "  ✓ .vscode/settings.json (auto-format on save)"
echo "  ✓ format.sh (quality check script)"

echo -e "\n${GREEN}Next steps:${NC}"
echo "  1. Install VS Code extension: xaver.clang-format"
echo "  2. Reload VS Code (Ctrl+Shift+P → Reload Window)"
echo "  3. Code normally - files auto-format on save"
echo "  4. Before committing: ./format.sh"

echo -e "\n${GREEN}Quick commands:${NC}"
echo "  ./format.sh          # Run all quality checks"
echo "  clang-format -i src/*.cpp  # Format one file"
echo "  cppcheck src/        # Find issues"

echo ""
