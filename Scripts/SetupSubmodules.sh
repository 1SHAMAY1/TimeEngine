#!/bin/bash
# SetupSubmodules.sh
# Initializes, cleans up, and recursively updates Git submodules to prevent compile issues with empty vendor folders.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "[=== Setting up Git Submodules ===]"
cd "$ROOT_DIR"

if [ ! -d ".git" ]; then
    echo "[ERROR] This script must be run inside a Git repository. .git folder not found."
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[INFO] Cleaning empty vendor folders to prevent Git update blockages..."
# Find and delete any empty directories inside Vendor to prevent git from blocking updates
if [ -d "Vendor" ]; then
    find Vendor -mindepth 1 -maxdepth 2 -type d -empty -delete 2>/dev/null
fi

echo "[INFO] Syncing submodule URLs..."
git submodule sync --recursive

echo "[INFO] Initializing submodules..."
git submodule init

echo "[INFO] Updating submodules recursively..."
git submodule update --init --recursive --force

if [ $? -eq 0 ]; then
    echo "[SUCCESS] Submodules successfully configured and updated!"
else
    echo "[ERROR] Failed to update submodules. Please check your internet connection and git configuration."
    read -p "Press Enter to exit..."
    exit 1
fi

read -p "Press Enter to continue..."
