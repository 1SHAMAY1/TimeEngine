#!/bin/bash
# GenerateProjectFiles.sh (Linux)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "[≡ Generating TimeEngine Workspace: Clean + CMake + Premake]"

# Inline Clean
echo "[🧹 Cleaning previous build artifacts, CMake, and project files...]"
rm -rf "$ROOT_DIR/Bin"
rm -rf "$ROOT_DIR/Bin-Intermediate"
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/build"
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/bin"
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/lib"
rm -f "$ROOT_DIR/Vendor/Customizable_Logger/CMakeCache.txt"
rm -rf "$ROOT_DIR/Vendor/Customizable_Logger/CMakeFiles"
rm -f "$ROOT_DIR/Vendor/Customizable_Logger/Makefile"
rm -f "$ROOT_DIR/Vendor/Customizable_Logger/cmake_install.cmake"
rm -rf "$ROOT_DIR/Vendor/GLFW/build"
rm -rf "$ROOT_DIR/Vendor/bin"
rm -f "$ROOT_DIR/Vendor/GLFW/CMakeCache.txt"
rm -rf "$ROOT_DIR/Vendor/GLFW/CMakeFiles"
rm -f "$ROOT_DIR/Vendor/GLFW/Makefile"
rm -f "$ROOT_DIR/Vendor/GLFW/cmake_install.cmake"
find "$ROOT_DIR" -type f \( -name "*.sln" -o -name "*.vcxproj" -o -name "*.vcxproj.filters" -o -name "*.vcxproj.user" -o -name "Makefile" -o -name "*.make" \) -delete
rm -rf "$ROOT_DIR/.vs"

echo "[✅ Cleanup complete.]"

# Logger
echo "[≡ CMake configure/build: Logger]"
mkdir -p "$ROOT_DIR/Vendor/Customizable_Logger/build"
cd "$ROOT_DIR/Vendor/Customizable_Logger/build"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POSITION_INDEPENDENT_CODE=ON
if [ $? -ne 0 ]; then
    echo "[✖ Logger CMake configuration failed.]"
    read -p "Press Enter to exit..."
    exit 1
fi
cmake --build . --config Debug
if [ $? -ne 0 ]; then
    echo "[✖ Logger build failed.]"
    read -p "Press Enter to exit..."
    exit 1
fi

# GLFW
echo "[≡ CMake configure/build: GLFW]"
mkdir -p "$ROOT_DIR/Vendor/GLFW/build"
cd "$ROOT_DIR/Vendor/GLFW/build"
cmake ../glfw -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POSITION_INDEPENDENT_CODE=ON
if [ $? -ne 0 ]; then
    echo "[✖ GLFW CMake configuration failed.]"
    read -p "Press Enter to exit..."
    exit 1
fi
cmake --build . --config Debug
if [ $? -ne 0 ]; then
    echo "[✖ GLFW build failed.]"
    read -p "Press Enter to exit..."
    exit 1
fi

# Premake
echo "[≡ Generating Makefiles with Premake...]"
cd "$ROOT_DIR"

if command -v premake5 &> /dev/null; then
    premake5 gmake2
else
    echo "[!] premake5 not found in PATH. Please install premake5 or ensure it is in your PATH."
    read -p "Press Enter to exit..."
    exit 1
fi

if [ $? -ne 0 ]; then
    echo "[✖ Premake generation failed.]"
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[✅ Setup complete. Run 'make' from root to begin development.]"
read -p "Press Enter to continue..."
