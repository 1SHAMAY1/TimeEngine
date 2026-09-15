#!/bin/bash
# SetupSubmodules.sh (Linux)
# Initializes Git submodules, installs Linux build & graphics dependencies, and downloads Premake 5.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "[=== Setting up TimeEngine Submodules, Dependencies & Tools (Linux) ===]"
cd "$ROOT_DIR"

if [ ! -d ".git" ]; then
    echo "[ERROR] This script must be run inside a Git repository. .git folder not found."
    read -p "Press Enter to exit..."
    exit 1
fi

# 1. Check and install system development packages
if command -v apt-get &> /dev/null; then
    echo "[INFO] Checking required Linux development packages (X11, OpenGL, Vulkan, ALSA)..."
    MISSING_PKGS=()
    PKGS=(
        libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxxf86vm-dev xorg-dev
        libgl1-mesa-dev libglu1-mesa-dev libvulkan-dev vulkan-tools
        libasound2-dev libpulse-dev zenity mold
    )

    for pkg in "${PKGS[@]}"; do
        if ! dpkg -s "$pkg" &> /dev/null; then
            MISSING_PKGS+=("$pkg")
        fi
    done

    if [ ${#MISSING_PKGS[@]} -gt 0 ]; then
        echo "[INFO] Installing missing Linux dependencies: ${MISSING_PKGS[*]}"
        if [ "$EUID" -ne 0 ]; then
            sudo apt-get update && sudo apt-get install -y "${MISSING_PKGS[@]}"
        else
            apt-get update && apt-get install -y "${MISSING_PKGS[@]}"
        fi
    else
        echo "[SUCCESS] All Linux graphics and audio dev packages are already installed!"
    fi
fi

# 2. Submodules
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

# 3. Premake 5 Linux Binary Download
PREMAKE_TAG="v5.0.0-beta4"
PREMAKE_VER="5.0.0-beta4"
PREMAKE_DIR="$ROOT_DIR/ThirdParty/Premake/Linux"
PREMAKE_EXE="$PREMAKE_DIR/premake5"
URL="https://github.com/premake/premake-core/releases/download/${PREMAKE_TAG}/premake-${PREMAKE_VER}-linux.tar.gz"

if [ ! -f "$PREMAKE_EXE" ]; then
    echo "[INFO] Downloading Premake 5 for Linux..."
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

echo ""
echo "[SUCCESS] Setup complete! You can now run GenerateProjectFiles.sh (GCC or Clang) to build the workspace."
read -p "Press Enter to continue..."
