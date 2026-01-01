@echo off
setlocal

set EXE=%~dp0LiveBanner.exe
if not exist "%EXE%" (
  echo LiveBanner.exe bulunamadi.
  exit /b 1
)

"%EXE%" install
if errorlevel 1 (
  echo Kurulumda hata olustu.
  exit /b 1
)

echo Kurulum tamamlandi. Yeni bir terminal acin.
endlocal
