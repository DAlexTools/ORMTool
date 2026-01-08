@echo off
echo Cleaning build directory...

cd ..
if exist build (
    rmdir /S /Q build
    echo Build directory deleted.
) else (
    echo Build directory does not exist. Skipping.
)

echo Clean complete.