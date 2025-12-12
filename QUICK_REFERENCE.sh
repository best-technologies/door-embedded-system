#!/usr/bin/env bash
# C++ Quality Tools - Quick Reference Card
# Print this or save as reference

cat << 'EOF'

╔════════════════════════════════════════════════════════════════════════╗
║                 C++ CODE QUALITY - QUICK REFERENCE                    ║
╚════════════════════════════════════════════════════════════════════════╝

┌─ FOR NEW PROJECTS (One-time Setup) ──────────────────────────────────┐
│                                                                        │
│  cd /path/to/new/project                                             │
│  bash ~/best_tech_door/setup-cpp-quality.sh                          │
│                                                                        │
│  OR copy template files:                                             │
│  cp -r ~/best_tech_door/{.clang-format,.clang-tidy,format.sh} .     │
│  mkdir -p .vscode                                                    │
│  cp ~/best_tech_door/.vscode/settings.json .vscode/                │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌─ DAILY WORKFLOW ─────────────────────────────────────────────────────┐
│                                                                        │
│  1. Code normally in VS Code                                         │
│     (Auto-formats on save ✓)                                         │
│                                                                        │
│  2. Before committing:                                               │
│     ./format.sh                                                      │
│     pio run                    (Compile)                             │
│     git add .                                                        │
│     git commit -m "message"                                          │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌─ MANUAL COMMANDS ────────────────────────────────────────────────────┐
│                                                                        │
│  Format all code:                                                    │
│  $ clang-format -i src/*.cpp include/*.h                            │
│                                                                        │
│  Check for issues:                                                   │
│  $ cppcheck --enable=all src/ include/                              │
│                                                                        │
│  Run linting:                                                        │
│  $ clang-tidy src/main.cpp -- -I./include/                          │
│                                                                        │
│  Run complete pipeline:                                              │
│  $ ./format.sh                                                       │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌─ KEY FILES & WHAT THEY DO ───────────────────────────────────────────┐
│                                                                        │
│  .clang-format       → Formatting rules (spacing, braces, etc)      │
│  .clang-tidy         → Linting rules (finds bugs, improvements)     │
│  format.sh           → Master script (runs formatting + checks)     │
│  .vscode/settings.json → VS Code auto-format settings              │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌─ INSTALL VS CODE EXTENSION ──────────────────────────────────────────┐
│                                                                        │
│  1. Open VS Code                                                     │
│  2. Ctrl+Shift+X (Extensions)                                       │
│  3. Search: xaver.clang-format                                      │
│  4. Click Install                                                    │
│  5. Ctrl+Shift+P → Reload Window                                    │
│                                                                        │
│  Now files auto-format on save! ✓                                    │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌─ FIRST TIME ONLY (Install Tools) ────────────────────────────────────┐
│                                                                        │
│  sudo apt-get update                                                 │
│  sudo apt-get install clang-format cppcheck clang-tidy              │
│                                                                        │
│  Verify:                                                             │
│  clang-format --version                                             │
│  cppcheck --version                                                 │
│  clang-tidy --version                                               │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌─ TROUBLESHOOTING ────────────────────────────────────────────────────┐
│                                                                        │
│  Problem: format.sh not executable                                   │
│  Fix: chmod +x format.sh                                            │
│                                                                        │
│  Problem: clang-format not found                                     │
│  Fix: sudo apt-get install clang-format                             │
│                                                                        │
│  Problem: VS Code not auto-formatting                                │
│  Fix: Reload extension (Ctrl+Shift+P → Reload Window)              │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌─ TEMPLATE FILES TO COPY ─────────────────────────────────────────────┐
│                                                                        │
│  Location: ~/Documents/challenge/Btech_door/best_tech_door/         │
│                                                                        │
│  Copy these to new projects:                                         │
│                                                                        │
│  • .clang-format                                                     │
│  • .clang-tidy                                                       │
│  • format.sh                                                         │
│  • setup-cpp-quality.sh (for complete setup)                        │
│  • .vscode/settings.json                                            │
│                                                                        │
│  OR just run:                                                        │
│  bash ~/best_tech_door/setup-cpp-quality.sh                         │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

Questions? Check these files:
  • CPP_TEMPLATE_README.md       (Full template documentation)
  • CODE_QUALITY_GUIDE.md         (Detailed setup guide)
  • SETUP_GUIDE.md                (Setup instructions for new projects)

EOF
