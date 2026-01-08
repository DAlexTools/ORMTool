#!/bin/bash

echo "====================================================="
echo "   Select build configuration:"
echo "   [1] Debug"
echo "   [2] Release"
echo "====================================================="

read -p "Enter your choice (1 or 2): " choice

if [ "$choice" == "1" ]; then
    CONFIG="Debug"
elif [ "$choice" == "2" ]; then
    CONFIG="Release"
else
    echo "Invalid choice."
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(realpath "$SCRIPT_DIR/..")"
BUILD_DIR="$PROJECT_ROOT/build"

echo
echo "====================================================="
echo "Configuration:    $CONFIG"
echo "Build directory:  $BUILD_DIR"
echo "====================================================="

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

cmake .. -DCMAKE_BUILD_TYPE=$CONFIG
if [ $? -ne 0 ]; then
    echo "CMake configuration failed."
    exit 1
fi


cmake --build . 
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo
echo "$CONFIG build completed successfully."
