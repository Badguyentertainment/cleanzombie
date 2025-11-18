@echo off
REM Batch file wrapper for PowerShell script
REM Run this as Administrator

echo.
echo ===================================================
echo   NU1701 Error Fix for Unreal Engine Projects
echo ===================================================
echo.
echo Starting PowerShell script...
echo.

PowerShell -NoProfile -ExecutionPolicy Bypass -Command "& '%~dp0Fix-NU1701-AllProjects.ps1'"

echo.
echo Press any key to exit...
pause > nul
