@echo off
setlocal

echo ============================================
echo  Building "KeyboardTrainer"
echo ============================================
echo.

set GCC=gcc
set GXX=g++
set WINDRES=windres
set OUT=build\KeyboardTrainer.exe

if not exist "build" mkdir build

echo [1/5] Compiling resources (icon)...
%WINDRES% resources\app.rc -O coff -o build\app.res
if errorlevel 1 (
    echo [FAIL] Resource compilation error
    pause
    exit /b 1
)
echo [OK] app.res built

echo [2/5] Compiling sqlite3.c (C)...
%GCC% -c -O2 -DSQLITE_THREADSAFE=0 lib\sqlite3.c -o lib\sqlite3.o
if errorlevel 1 (
    echo [FAIL] sqlite3.c compilation error
    pause
    exit /b 1
)
echo [OK] sqlite3.o built

echo [3/5] Compiling C++ sources...
%GXX% ^
    -std=c++17 ^
    -O2 ^
    -mwindows ^
    -static ^
    -static-libgcc ^
    -static-libstdc++ ^
    -s ^
    src\main.cpp ^
    src\MainWindow.cpp ^
    src\Exercise.cpp ^
    src\ExerciseFactory.cpp ^
    src\DatabaseManager.cpp ^
    src\Settings.cpp ^
    src\FlatButton.cpp ^
    lib\sqlite3.o ^
    build\app.res ^
    -o %OUT% ^
    -luser32 -lgdi32 -lcomctl32 -lwinmm -luxtheme

if errorlevel 1 (
    echo.
    echo [FAIL] Compilation error
    pause
    exit /b 1
)
echo [OK] Compilation successful

echo [4/5] Copying resources...
copy /Y resources\error.wav build\error.wav >nul
if errorlevel 1 (
    echo [WARN] error.wav not found
) else (
    echo [OK] error.wav copied
)

echo [5/5] Checking size...
for %%A in ("%OUT%") do echo Size: %%~zA bytes

echo.
echo ============================================
echo  Build completed successfully
echo  File: %OUT%
echo ============================================
pause
endlocal