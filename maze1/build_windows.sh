#!/bin/bash
# Cross-compile Qt app for Windows (static build required)
# You must have MinGW-w64 and static Qt for Windows installed

# Set these paths to your static Qt for Windows and MinGW-w64
QT_STATIC_PATH=/opt/qt-static-win
MINGW_PATH=/usr/bin/x86_64-w64-mingw32

# Clean previous build
rm -rf build-win/
mkdir -p build-win

# Configure for cross-compiling
cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake -DCMAKE_PREFIX_PATH=$QT_STATIC_PATH -DCMAKE_BUILD_TYPE=Release .

# Build
cmake --build build-win --target Maze1 -- -j$(nproc)

# Output executable
mkdir -p dist-win
cp build-win/Maze1.exe dist-win/

echo "Windows executable is in dist-win/Maze1.exe"
echo "You must use a static Qt build for true portability."
