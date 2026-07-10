#!/bin/bash
# CleanProjectFiles.sh (Mac)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "[🧹 Cleaning TimeEngine build artifacts, CMake, and project files...]"

# Root build folders
rm -rf "$ROOT_DIR/Bin"
rm -rf "$ROOT_DIR/Bin-Intermediate"

# Logger CMake cleanup
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/build"
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/bin"
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/lib"
rm -f "$ROOT_DIR/Vendor/Customizable_Logger/CMakeCache.txt"
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/CMakeFiles"
rm -f "$ROOT_DIR/Vendor/Customizable_Logger/Makefile"
rm -f "$ROOT_DIR/Vendor/Customizable_Logger/cmake_install.cmake"

# GLFW CMake cleanup
rm -rf "$ROOT_DIR/Vendor/GLFW/build"
rm -rf "$ROOT_DIR/Vendor/bin"
rm -f "$ROOT_DIR/Vendor/GLFW/CMakeCache.txt"
rm -rf "$ROOT_DIR/Vendor/GLFW/CMakeFiles"
rm -f "$ROOT_DIR/Vendor/GLFW/Makefile"
rm -f "$ROOT_DIR/Vendor/GLFW/cmake_install.cmake"

# VS/Premake/Xcode generated files
find "$ROOT_DIR" -type f \( -name "*.sln" -o -name "*.vcxproj" -o -name "*.vcxproj.filters" -o -name "*.vcxproj.user" -o -name "Makefile" -o -name "*.make" \) -delete
rm -rf "$ROOT_DIR/.vs"
rm -rf "$ROOT_DIR"/*.xcodeproj
rm -rf "$ROOT_DIR"/*.xcworkspace

echo "[✅ Cleanup complete. All build, CMake, and project artifacts removed.]"
read -p "Press Enter to continue..."
