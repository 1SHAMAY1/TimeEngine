#!/bin/bash
# SetupSubmodules.sh
# Initializes Git submodules and downloads platform-specific Premake 5 binary if missing.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "[=== Setting up TimeEngine Submodules & Tools ===]"
cd "$ROOT_DIR"

if [ ! -d ".git" ]; then
    echo "[ERROR] This script must be run inside a Git repository. .git folder not found."
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[INFO] Cleaning empty vendor folders to prevent Git update blockages..."
if [ -d "Vendor" ]; then
    find Vendor -mindepth 1 -maxdepth 2 -type d -empty -delete 2>/dev/null
fi

echo "[INFO] Syncing submodule URLs..."
git submodule sync --recursive

echo "[INFO] Initializing submodules..."
git submodule init

echo "[INFO] Updating submodules recursively..."
git submodule update --init --recursive --force

if [ $? -ne 0 ]; then
    echo "[ERROR] Failed to update submodules. Please check your internet connection and git configuration."
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[SUCCESS] Git submodules updated successfully!"

# ========== Premake 5 Binary Download ==========
OS_NAME="$(uname -s)"
PREMAKE_VER="v5.0.0-beta2"

if [ "$OS_NAME" = "Darwin" ]; then
    PREMAKE_DIR="$ROOT_DIR/Vendor/Premake/Mac"
    PREMAKE_EXE="$PREMAKE_DIR/premake5"
    URL="https://github.com/premake/premake-core/releases/download/${PREMAKE_VER}/premake-5.0.0-beta2-macosx.tar.gz"
elif [ "$OS_NAME" = "Linux" ]; then
    PREMAKE_DIR="$ROOT_DIR/Vendor/Premake/Linux"
    PREMAKE_EXE="$PREMAKE_DIR/premake5"
    URL="https://github.com/premake/premake-core/releases/download/${PREMAKE_VER}/premake-5.0.0-beta2-linux.tar.gz"
else
    PREMAKE_DIR="$ROOT_DIR/Vendor/Premake/Windows"
    PREMAKE_EXE="$PREMAKE_DIR/premake5.exe"
    URL="https://github.com/premake/premake-core/releases/download/${PREMAKE_VER}/premake-5.0.0-beta2-windows.zip"
fi

if [ ! -f "$PREMAKE_EXE" ]; then
    if [ -n "$URL" ]; then
        echo "[INFO] Downloading Premake 5 for ${OS_NAME}..."
        mkdir -p "$PREMAKE_DIR"
        if [[ "$URL" == *.zip ]]; then
            curl -sSL "$URL" -o "$PREMAKE_DIR/premake.zip"
            if command -v unzip &> /dev/null; then
                unzip -q -o "$PREMAKE_DIR/premake.zip" -d "$PREMAKE_DIR"
            else
                powershell -Command "Expand-Archive -Path '${PREMAKE_DIR}/premake.zip' -DestinationPath '${PREMAKE_DIR}' -Force"
            fi
            rm -f "$PREMAKE_DIR/premake.zip"
        else
            curl -sSL "$URL" | tar -xz -C "$PREMAKE_DIR"
            chmod +x "$PREMAKE_EXE" 2>/dev/null
        fi
        if [ -f "$PREMAKE_EXE" ]; then
            echo "[SUCCESS] Premake 5 successfully installed to $PREMAKE_EXE"
        else
            echo "[WARN] Could not download Premake 5 binary. System premake5 will be used if available."
        fi
    fi
else
    echo "[INFO] Premake 5 binary verified at $PREMAKE_EXE"
fi

echo "[SUCCESS] Setup complete! You can now run GenerateProjectFiles to build the workspace."
read -p "Press Enter to continue..."
