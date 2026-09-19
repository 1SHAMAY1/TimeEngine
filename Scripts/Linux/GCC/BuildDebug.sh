#!/bin/bash
# BuildDebug.sh (Linux GCC)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../../.." && pwd )"

echo "[=== Building TimeEngine (Linux GCC Debug) ===]"
cd "$ROOT_DIR"

if [ ! -f "Makefile" ]; then
    echo "[ERROR] Makefile not found. Please run GenerateProjectFiles.sh first."
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[INFO] Running make with GCC..."
BUILD_LOG=$(mktemp)
make config=debug -j$(nproc 2>/dev/null || echo 2) CC=gcc CXX=g++ 2>&1 | tee "$BUILD_LOG"
BUILD_STATUS=${PIPESTATUS[0]}

WARNING_COUNT=$(grep -c -i "warning:" "$BUILD_LOG" 2>/dev/null)
WARNING_COUNT=${WARNING_COUNT:-0}
ERROR_COUNT=$(grep -c -i "error:" "$BUILD_LOG" 2>/dev/null)
ERROR_COUNT=${ERROR_COUNT:-0}
rm -f "$BUILD_LOG"

echo ""
echo "=================================================="
echo "  Build Summary: GCC (Debug)"
echo "--------------------------------------------------"
echo "  Total Warnings: $WARNING_COUNT"
echo "  Total Errors:   $ERROR_COUNT"
echo "=================================================="

if [ $BUILD_STATUS -ne 0 ]; then
    echo "[ERROR] Build failed!"
    read -p "Press Enter to exit..."
    exit $BUILD_STATUS
fi

echo "[SUCCESS] Build completed successfully with GCC."
read -p "Press Enter to continue..."
