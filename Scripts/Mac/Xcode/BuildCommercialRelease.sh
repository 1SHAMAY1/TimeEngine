#!/bin/bash
# BuildCommercialRelease.sh (Mac Xcode)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../../.." && pwd )"

echo "[=== Building TimeEngine Commercial Release (Dist) ===]"

cd "$ROOT_DIR"

if [ -d "TimeEngine.xcworkspace" ]; then
    echo "[≡ Building project using xcodebuild (workspace)...]"
    DEVELOPER_DIR="/Applications/Xcode.app/Contents/Developer" xcodebuild -workspace TimeEngine.xcworkspace -scheme TimeEditor -configuration Dist
elif [ -d "TimeEngine.xcodeproj" ]; then
    echo "[≡ Building project using xcodebuild (project)...]"
    DEVELOPER_DIR="/Applications/Xcode.app/Contents/Developer" xcodebuild -project TimeEngine.xcodeproj -scheme TimeEditor -configuration Dist
else
    echo "[!] Xcode project or workspace not found. Please run GenerateProjectFiles.sh first."
    read -p "Press Enter to exit..."
    exit 1
fi

if [ $? -ne 0 ]; then
    echo "[✖ Build Failed!]"
    read -p "Press Enter to exit..."
    exit 1
fi

echo ""
echo "[✅ Build Successful!]"

DIST_DIR="$ROOT_DIR/Build_Release"
echo "[≡ Packaging to: $DIST_DIR]"

rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Find build output
BUILD_OUTPUT="$ROOT_DIR/Bin/Dist-macosx-x86_64/TimeEditor"
if [ ! -d "$BUILD_OUTPUT" ]; then
    BUILD_OUTPUT=$(find "$ROOT_DIR/Bin" -type d -path "*/Dist-*/TimeEditor" | head -n 1)
fi

if [ -z "$BUILD_OUTPUT" ] || { [ ! -d "$BUILD_OUTPUT/TimeEditor.app" ] && [ ! -f "$BUILD_OUTPUT/TimeEditor" ]; }; then
    echo "[!] Error: Build output not found in $BUILD_OUTPUT"
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[  - Packaging TimeEditor.app Bundle...]"
if [ -d "$BUILD_OUTPUT/TimeEditor.app" ]; then
    cp -R "$BUILD_OUTPUT/TimeEditor.app" "$DIST_DIR/"
else
    cp -R "$BUILD_OUTPUT/"* "$DIST_DIR/"
fi

echo "[  - Copying Assets & Resources...]"
if [ -d "$ROOT_DIR/Resources" ]; then
    cp -R "$ROOT_DIR/Resources" "$DIST_DIR/"
    if [ -d "$DIST_DIR/TimeEditor.app" ]; then
        cp -R "$ROOT_DIR/Resources" "$DIST_DIR/TimeEditor.app/Contents/"
    fi
fi

# Ensure dylibs are inside TimeEditor.app/Contents/MacOS/ and rpath is correct
ENGINE_DIR=$(find "$ROOT_DIR/Bin" -type d -path "*/Dist-*/Engine" | head -n 1)
VELOX_DIR=$(find "$ROOT_DIR/Bin" -type d -path "*/Dist-*/Velox" | head -n 1)

if [ -d "$DIST_DIR/TimeEditor.app" ]; then
    if [ -n "$ENGINE_DIR" ] && [ -f "$ENGINE_DIR/libEngine.dylib" ]; then
        cp -f "$ENGINE_DIR/libEngine.dylib" "$DIST_DIR/TimeEditor.app/Contents/MacOS/"
    fi
    if [ -n "$VELOX_DIR" ] && [ -f "$VELOX_DIR/libVelox.dylib" ]; then
        cp -f "$VELOX_DIR/libVelox.dylib" "$DIST_DIR/TimeEditor.app/Contents/MacOS/"
    fi

    # Fix install_name and rpath for standalone bundle
    install_name_tool -id "@rpath/libEngine.dylib" "$DIST_DIR/TimeEditor.app/Contents/MacOS/libEngine.dylib" 2>/dev/null || true
    install_name_tool -id "@rpath/libVelox.dylib" "$DIST_DIR/TimeEditor.app/Contents/MacOS/libVelox.dylib" 2>/dev/null || true
    install_name_tool -change "/usr/local/lib/libEngine.dylib" "@rpath/libEngine.dylib" "$DIST_DIR/TimeEditor.app/Contents/MacOS/TimeEditor" 2>/dev/null || true
    install_name_tool -change "/usr/local/lib/libVelox.dylib" "@rpath/libVelox.dylib" "$DIST_DIR/TimeEditor.app/Contents/MacOS/TimeEditor" 2>/dev/null || true
    install_name_tool -add_rpath "@executable_path/" "$DIST_DIR/TimeEditor.app/Contents/MacOS/TimeEditor" 2>/dev/null || true
    codesign --force --deep --sign - "$DIST_DIR/TimeEditor.app" 2>/dev/null || true
fi

# Create a Run script for convenience
echo "#!/bin/bash" > "$DIST_DIR/RunEngine.sh"
echo 'SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"' >> "$DIST_DIR/RunEngine.sh"
echo 'open "$SCRIPT_DIR/TimeEditor.app"' >> "$DIST_DIR/RunEngine.sh"
chmod +x "$DIST_DIR/RunEngine.sh"

echo ""
echo "[✅ Packaging Complete!]"
echo "[Executable App Bundle located at: $DIST_DIR/TimeEditor.app]"
echo "[You can now package the '$DIST_DIR' folder and distribute it.]"
echo ""
read -p "Press Enter to continue..."
