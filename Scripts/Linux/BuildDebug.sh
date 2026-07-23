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
