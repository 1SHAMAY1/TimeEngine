#!/bin/bash
# BuildCommercialRelease.sh (Mac)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "[≡ Building TimeEngine Commercial Release (Dist)]"

cd "$ROOT_DIR"

if [ -d "TimeEngine.xcworkspace" ]; then
    echo "[≡ Building project using xcodebuild (workspace)...]"
    xcodebuild -workspace TimeEngine.xcworkspace -scheme TimeEditor -configuration Dist
elif [ -d "TimeEngine.xcodeproj" ]; then
    echo "[≡ Building project using xcodebuild (project)...]"
    xcodebuild -project TimeEngine.xcodeproj -scheme TimeEditor -configuration Dist
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

if [ -z "$BUILD_OUTPUT" ] || [ ! -f "$BUILD_OUTPUT/TimeEditor" ]; then
    echo "[!] Error: Build output not found at $BUILD_OUTPUT/TimeEditor"
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[  - Copying Binaries...]"
cp -R "$BUILD_OUTPUT/"* "$DIST_DIR/"

echo "[  - Copying Assets & Resources...]"
if [ -d "$ROOT_DIR/Resources" ]; then
    cp -R "$ROOT_DIR/Resources" "$DIST_DIR/"
else
    echo "[!] Warning: Resources folder not found at $ROOT_DIR/Resources"
fi

# Create a Run script for convenience
echo "#!/bin/bash" > "$DIST_DIR/RunEngine.sh"
echo "./TimeEditor" >> "$DIST_DIR/RunEngine.sh"
chmod +x "$DIST_DIR/RunEngine.sh"

echo ""
echo "[✅ Packaging Complete!]"
echo "[Executable located at: $DIST_DIR/TimeEditor]"
echo "[You can now package the '$DIST_DIR' folder and distribute it.]"
echo ""
read -p "Press Enter to continue..."
