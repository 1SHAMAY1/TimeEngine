#!/bin/bash
# RegisterFileExtension.sh (Mac Makefiles)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../../.." && pwd )"

# Search for TimeEditor executable
ENGINE_EXE=""
for path in \
    "Bin/Debug-macosx-x86_64/TimeEditor/TimeEditor" \
    "Bin/Release-macosx-x86_64/TimeEditor/TimeEditor" \
    "Bin/Debug-macosx-arm64/TimeEditor/TimeEditor" \
    "Bin/Release-macosx-arm64/TimeEditor/TimeEditor"; do
    if [ -f "$ROOT_DIR/$path" ]; then
        ENGINE_EXE="$ROOT_DIR/$path"
        break
    fi
done

if [ -z "$ENGINE_EXE" ]; then
    ENGINE_EXE=$(find "$ROOT_DIR/Bin" -type f -name "TimeEditor" | grep -v "Intermediate" | grep -v "\.dSYM" | grep -v "\.app" | head -n 1)
fi

if [ -n "$ENGINE_EXE" ] && [ -f "$ENGINE_EXE" ]; then
    echo "[TimeEngine] Packaging TimeEditor into macOS Application Bundle (.app)..."
    echo "[TimeEngine] Found executable: $ENGINE_EXE"
    chmod +x "$ENGINE_EXE"

    APP_DIR="$(dirname "$ENGINE_EXE")"
    BUNDLE_DIR="$APP_DIR/TimeEditor.app"
    CONTENTS_DIR="$BUNDLE_DIR/Contents"
    MACOS_DIR="$CONTENTS_DIR/MacOS"
    RESOURCES_DIR="$CONTENTS_DIR/Resources"

    mkdir -p "$MACOS_DIR"
    mkdir -p "$RESOURCES_DIR"

    # Create Info.plist inside bundle
    cat << 'EOF' > "$CONTENTS_DIR/Info.plist"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>TimeEditor</string>
    <key>CFBundleIdentifier</key>
    <string>com.timeengine.timeeditor</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>TimeEditor</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.14</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>CFBundleDocumentTypes</key>
    <array>
        <dict>
            <key>CFBundleTypeExtensions</key>
            <array>
                <string>teproj</string>
            </array>
            <key>CFBundleTypeName</key>
            <string>TimeEngine Project</string>
            <key>CFBundleTypeRole</key>
            <string>Editor</string>
            <key>LSHandlerRank</key>
            <string>Owner</string>
        </dict>
    </array>
</dict>
</plist>
EOF

    # Copy binary into bundle
    cp "$ENGINE_EXE" "$MACOS_DIR/TimeEditor"
    chmod +x "$MACOS_DIR/TimeEditor"

    # Copy dynamic libraries if present
    find "$APP_DIR/.." -name "*.dylib" -exec cp {} "$MACOS_DIR/" \; 2>/dev/null || true

    # Ensure @executable_path/ is in rpath so dyld finds dylibs inside bundle
    install_name_tool -add_rpath "@executable_path/" "$MACOS_DIR/TimeEditor" 2>/dev/null || true

    # Ad-hoc code sign bundle so macOS Gatekeeper permits execution
    codesign --force --deep --sign - "$BUNDLE_DIR" 2>/dev/null || true

    echo "[TimeEngine] TimeEditor.app bundle created and signed at: $BUNDLE_DIR"

    # Register with macOS Launch Services if lsregister tool exists
    LSREGISTER="/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister"
    if [ -f "$LSREGISTER" ]; then
        echo "[TimeEngine] Registering TimeEditor.app with Launch Services..."
        "$LSREGISTER" -f "$BUNDLE_DIR"
    fi

    echo "[TimeEngine] Running internal registration..."
    DYLD_LIBRARY_PATH="$APP_DIR/../Engine:$APP_DIR/../Velox:$DYLD_LIBRARY_PATH" "$ENGINE_EXE" --register || true
    echo "[TimeEngine] Registration process completed successfully."
else
    echo "[TimeEngine] ERROR: TimeEditor executable not found!"
    echo "[TimeEngine] Please build the TimeEditor project first before running this script."
    read -p "Press Enter to continue..."
fi

