@echo off
echo Starting Durham Language Web Compiler...
echo.
echo Opening http://localhost:8000
echo Press Ctrl+C to stop the server
echo.

cd /d "%~dp0"
python -m http.server 8000

pause
