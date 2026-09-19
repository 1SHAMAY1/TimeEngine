#!/bin/bash
# GenerateProjectFiles.sh (Linux GCC)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../../.." && pwd )"

echo "[=== Generating TimeEngine Workspace: Clean + CMake + Premake (GCC) ===]"

# Inline Clean
echo "[🧹 Cleaning previous build artifacts, CMake, and project files...]"
rm -rf "$ROOT_DIR/Artifacts/Bin"
rm -rf "$ROOT_DIR/Artifacts/Bin-Intermediate"
rm -rf "$ROOT_DIR/ThirdParty/Customizable_Logger/build"
rm -rf "$ROOT_DIR/ThirdParty/Customizable_Logger/bin"
rm -rf "$ROOT_DIR/ThirdParty/Customizable_Logger/lib"
rm -f "$ROOT_DIR/ThirdParty/Customizable_Logger/CMakeCache.txt"
rm -rf "$ROOT_DIR/ThirdParty/Customizable_Logger/CMakeFiles"
rm -f "$ROOT_DIR/ThirdParty/Customizable_Logger/Makefile"
rm -f "$ROOT_DIR/ThirdParty/Customizable_Logger/cmake_install.cmake"
rm -rf "$ROOT_DIR/ThirdParty/GLFW/build"
rm -rf "$ROOT_DIR/ThirdParty/bin"
rm -f "$ROOT_DIR/ThirdParty/GLFW/CMakeCache.txt"
rm -rf "$ROOT_DIR/ThirdParty/GLFW/CMakeFiles"
rm -f "$ROOT_DIR/ThirdParty/GLFW/Makefile"
rm -f "$ROOT_DIR/ThirdParty/GLFW/cmake_install.cmake"
find "$ROOT_DIR" -type f \( -name "*.sln" -o -name "*.vcxproj" -o -name "*.vcxproj.filters" -o -name "*.vcxproj.user" -o -name "Makefile" -o -name "*.make" \) -delete
rm -rf "$ROOT_DIR/.vs"

echo "[✅ Cleanup complete.]"

# Logger
echo "[≡ CMake configure/build: Logger (GCC)]"
mkdir -p "$ROOT_DIR/ThirdParty/Customizable_Logger/build"
cd "$ROOT_DIR/ThirdParty/Customizable_Logger/build"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
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
echo "[≡ CMake configure/build: GLFW (GCC)]"
mkdir -p "$ROOT_DIR/ThirdParty/GLFW/build"
cd "$ROOT_DIR/ThirdParty/GLFW/build"
cmake ../glfw -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
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

PREMAKE_BIN=""
if [ -x "$ROOT_DIR/ThirdParty/Premake/Linux/premake5" ]; then
    PREMAKE_BIN="$ROOT_DIR/ThirdParty/Premake/Linux/premake5"
elif [ -x "$ROOT_DIR/ThirdParty/Premake/premake5" ]; then
    PREMAKE_BIN="$ROOT_DIR/ThirdParty/Premake/premake5"
elif command -v premake5 &> /dev/null; then
    PREMAKE_BIN="premake5"
fi

if [ -n "$PREMAKE_BIN" ]; then
    "$PREMAKE_BIN" gmake2 --file=Premake5.lua
else
    echo "[!] premake5 not found in ThirdParty/Premake/Linux/ or system PATH."
    echo "    Please install premake5 on your system or place it in ThirdParty/Premake/Linux/premake5."
    read -p "Press Enter to exit..."
    exit 1
fi

if [ $? -ne 0 ]; then
    echo "[✖ Premake generation failed.]"
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[✅ Setup complete. Run GCC/BuildDebug.sh or 'make CC=gcc CXX=g++' to compile.]"
read -p "Press Enter to continue..."
