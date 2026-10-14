
@echo off
chcp 65001 >nul
setlocal

echo ============================================
echo  Сборка тестов
echo ============================================
echo.

g++ ^
    -std=c++17 ^
    -O2 ^
    tests\test_runner.cpp ^
    src\Exercise.cpp ^
    src\ExerciseFactory.cpp ^
    src\Settings.cpp ^
    -o tests\test_runner.exe ^
    -luser32 -lgdi32 -lcomctl32

if errorlevel 1 (
    echo [FAIL] Ошибка сборки тестов
    pause
    exit /b 1
)

echo [OK] Тесты собраны: tests\test_runner.exe
pause
endlocal