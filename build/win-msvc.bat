@echo off
setlocal

rem Build with MSVC (cl.exe must be in PATH)
set EXE=LiveBanner.exe
set INCLUDES=/I include
set SOURCES=src\*.c

cl /nologo /W3 /O2 %INCLUDES% /Fe:%EXE% %SOURCES% /link /SUBSYSTEM:CONSOLE
if errorlevel 1 exit /b 1

echo Built %EXE%
endlocal