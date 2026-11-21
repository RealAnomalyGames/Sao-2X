@echo off
setlocal

echo ====================================
echo Sao-2X Build System
echo ====================================

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 goto error

cmake --build build --config Release

if errorlevel 1 goto error

ctest --test-dir build -C Release --output-on-failure

if errorlevel 1 goto error

echo.
echo Sao-2X build completed successfully.
exit /b 0

:error
echo.
echo Sao-2X build failed.
exit /b 1
