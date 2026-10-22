@echo off
setlocal

set PRESET=local
if not "%~1"=="" set PRESET=%~1

cmake --preset %PRESET%
if errorlevel 1 exit /b %errorlevel%

cmake --build --preset %PRESET%
if errorlevel 1 exit /b %errorlevel%

endlocal