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

echo "[INFO] Packaging dylibs and resources into TimeEditor.app..."
BUILD_DIR=$(find "$ROOT_DIR/Bin" -type d -path "*/Debug-*/TimeEditor" | head -n 1)
ENGINE_DIR=$(find "$ROOT_DIR/Bin" -type d -path "*/Debug-*/Engine" | head -n 1)
VELOX_DIR=$(find "$ROOT_DIR/Bin" -type d -path "*/Debug-*/Velox" | head -n 1)

if [ -d "$BUILD_DIR/TimeEditor.app" ]; then
    mkdir -p "$BUILD_DIR/TimeEditor.app/Contents/MacOS"
    if [ -n "$ENGINE_DIR" ] && [ -f "$ENGINE_DIR/libEngine.dylib" ]; then
        cp -f "$ENGINE_DIR/libEngine.dylib" "$BUILD_DIR/TimeEditor.app/Contents/MacOS/"
    fi
    if [ -n "$VELOX_DIR" ] && [ -f "$VELOX_DIR/libVelox.dylib" ]; then
        cp -f "$VELOX_DIR/libVelox.dylib" "$BUILD_DIR/TimeEditor.app/Contents/MacOS/"
    fi
    if [ -d "$ROOT_DIR/Resources" ]; then
        cp -R "$ROOT_DIR/Resources" "$BUILD_DIR/TimeEditor.app/Contents/"
    fi
    codesign --force --deep --sign - "$BUILD_DIR/TimeEditor.app" 2>/dev/null || true
fi

echo ""
echo "[SUCCESS] Build completed successfully."
read -p "Press Enter to continue..."
