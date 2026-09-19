#!/bin/bash
# BuildDebug.sh (Linux)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "[=== Building TimeEngine (Linux Debug) ===]"
cd "$ROOT_DIR"

if [ ! -f "Makefile" ]; then
    echo "[ERROR] Makefile not found. Please run GenerateProjectFiles.sh first."
    read -p "Press Enter to exit..."
    exit 1
fi

# Run TimeEngine code quality and architecture safety checks once upfront
if [ -f "ThirdParty/Premake/Linux/premake5" ]; then
    echo "[INFO] Running architecture and code safety rule check..."
    ThirdParty/Premake/Linux/premake5 --file="Premake5.lua" check-rules
    if [ $? -ne 0 ]; then
        echo "[ERROR] Rule check failed! Build aborted."
        read -p "Press Enter to exit..."
        exit 1
    fi
fi

echo "[INFO] Running make..."
make config=debug -j$(nproc 2>/dev/null || echo 2)
if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed!"
    read -p "Press Enter to exit..."
    exit 1
fi

echo ""
echo "[SUCCESS] Build completed successfully."
read -p "Press Enter to continue..."
