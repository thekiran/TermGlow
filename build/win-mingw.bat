@echo off
setlocal

rem Build with MinGW-w64 (gcc must be in PATH)
set EXE=LiveBanner.exe
set INCLUDES=-I include
set SOURCES=src\*.c

gcc -O2 -Wall -Wextra %INCLUDES% -o %EXE% %SOURCES% -ladvapi32 -lshell32
if errorlevel 1 exit /b 1

echo Built %EXE%
endlocal