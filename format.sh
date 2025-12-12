#!/bin/bash
# Complete C++ formatting and linting workflow

set -e  # Exit on error

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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

# Step 1: Format code
echo -e "\n${YELLOW}📝 Step 1: Formatting code with clang-format...${NC}"
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
echo -e "${GREEN}✓ Formatting complete${NC}"

# Step 2: Static analysis with cppcheck
echo -e "\n${YELLOW}🔎 Step 2: Running cppcheck (static analysis)...${NC}"
cppcheck --enable=all --include-path=include/ \
    --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    src/ include/ 2>&1 | tee cppcheck_report.txt

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Cppcheck passed${NC}"
else
    echo -e "${YELLOW}⚠ Cppcheck found issues (see cppcheck_report.txt)${NC}"
fi

# Step 3: Clang-tidy analysis
echo -e "\n${YELLOW}🔍 Step 3: Running clang-tidy (advanced linting)...${NC}"
if command -v clang-tidy &> /dev/null; then
    find src include -name "*.cpp" -o -name "*.h" | while read file; do
        echo "  Analyzing: $file"
        clang-tidy "$file" -- -I./include/ 2>&1 | head -20 || true
    done
    echo -e "${GREEN}✓ Clang-tidy analysis complete${NC}"
fi

# Step 4: Summary
echo -e "\n${YELLOW}═══════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}✅ Pipeline complete!${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════${NC}"
echo ""
echo "Files checked:"
echo "  - Source: src/*.cpp"
echo "  - Headers: include/*.h"
echo ""
echo "Reports generated:"
echo "  - cppcheck_report.txt"
echo ""
echo "Next steps:"
echo "  1. Review cppcheck_report.txt for issues"
echo "  2. Fix any critical warnings"
echo "  3. Run 'pio run' to compile"
echo ""
