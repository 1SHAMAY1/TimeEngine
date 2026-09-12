#!/bin/bash
# SetupSubmodules.sh (macOS)
# Initializes Git submodules and downloads macOS Premake 5 binary if missing.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "[=== Setting up TimeEngine Submodules & Tools (macOS) ===]"
cd "$ROOT_DIR"

if [ ! -d ".git" ]; then
    echo "[ERROR] This script must be run inside a Git repository. .git folder not found."
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[INFO] Syncing submodule URLs..."
git submodule sync --recursive

echo "[INFO] Initializing submodules..."
git submodule init

echo "[INFO] Updating submodules recursively from remote..."
git submodule update --init --recursive --remote

if [ $? -ne 0 ]; then
    echo "[ERROR] Failed to update submodules. Please check your internet connection and git configuration."
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[SUCCESS] Git submodules updated successfully!"

# Premake 5 macOS Binary Download
PREMAKE_VER="v5.0.0-beta2"
PREMAKE_DIR="$ROOT_DIR/Vendor/Premake/Mac"
PREMAKE_EXE="$PREMAKE_DIR/premake5"
URL="https://github.com/premake/premake-core/releases/download/${PREMAKE_VER}/premake-5.0.0-beta2-macosx.tar.gz"

if [ ! -f "$PREMAKE_EXE" ]; then
    echo "[INFO] Downloading Premake 5 for macOS..."
    mkdir -p "$PREMAKE_DIR"
    curl -sSL "$URL" | tar -xz -C "$PREMAKE_DIR"
    chmod +x "$PREMAKE_EXE" 2>/dev/null
    if [ -f "$PREMAKE_EXE" ]; then
        echo "[SUCCESS] Premake 5 successfully installed to $PREMAKE_EXE"
    else
        echo "[WARN] Could not download Premake 5 binary. System premake5 will be used if available."
    fi
else
    echo "[INFO] Premake 5 binary verified at $PREMAKE_EXE"
fi

echo "[SUCCESS] Setup complete! You can now run GenerateProjectFiles.sh to build the workspace."
