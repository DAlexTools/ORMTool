@echo off
echo Configuring project with CMake...

cd ..
if not exist build (
    mkdir build
)
cd build
cmake ..
cd ../dev_ops

echo  Configuration complete.