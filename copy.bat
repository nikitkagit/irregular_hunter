@echo off
setlocal

REM ==== Настройки ====
set "SCRIPT=C:\Dev\PythonScripts\merge_to_buffer.py"
set "INPUT_DIR=%~dp0"
set "INPUT_DIR=%INPUT_DIR:~0,-1%"

REM ==== Белый список: папки и файлы (через пробел) ====
set "WHITELIST=src include test example"

REM ==== Запуск ====
python "%SCRIPT%" "%INPUT_DIR%" %WHITELIST%

if errorlevel 1 (
    echo.
    echo Ошибка при выполнении скрипта.
    pause
    exit /b 1
)

echo.
echo Готово.
endlocal