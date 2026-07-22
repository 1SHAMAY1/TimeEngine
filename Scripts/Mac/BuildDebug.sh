#!/bin/bash
# BuildDebug.sh (Mac)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "[=== Building TimeEngine (Mac Debug via xcodebuild) ===]"
cd "$ROOT_DIR"

WORKSPACE_FILE=$(find . -maxdepth 1 -name "*.xcworkspace" | head -n 1)

if [ -z "$WORKSPACE_FILE" ]; then
    echo "[ERROR] Xcode Workspace not found. Please run GenerateProjectFiles.sh first."
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[INFO] Running xcodebuild..."
xcodebuild -workspace "$WORKSPACE_FILE" -scheme TimeEditor -configuration Debug -parallelizeTargets
if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed!"
    read -p "Press Enter to exit..."
    exit 1
fi

echo ""
echo "[SUCCESS] Build completed successfully."
read -p "Press Enter to continue..."
