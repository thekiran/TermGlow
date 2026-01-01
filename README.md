# LiveBanner

LiveBanner is a Windows console banner manager and runner. The manager lets you edit or remove the banner, and the runner prints the banner with a simple RGB animation while keeping the cursor usable.

<img width="932" height="554" alt="resim" src="https://github.com/user-attachments/assets/13727ffc-5c50-4548-ab6e-b05b9631073d" />
<img width="1110" height="615" alt="resim" src="https://github.com/user-attachments/assets/c64f79ac-71d4-4c5c-97c5-05c4dd20d824" />


## Download (Fast)
Option A: Installer (recommended)
1. Download `LiveBannerSetup.exe` from the Releases page.
2. Run it and finish the setup.
3. Open a new terminal and type `LiveBanner`.

Option B: Portable ZIP
1. Download the ZIP from the Releases page.
2. Extract it to a folder (for example `C:\LiveBanner`).
3. Run `install.bat` (or `LiveBanner.exe install`).
4. Open a new terminal and type `LiveBanner`.

## Build (Developers)
MSVC:
```
build\win-msvc.bat
```

MinGW-w64:
```
build\win-mingw.bat
```

## Use
Manager (default):
```
LiveBanner
```

Runner (one-shot):
```
LiveBanner run
```

Runner (loop):
```
LiveBanner run-loop
```

## Notes
- Banner file: `%APPDATA%\LiveBanner\banner.txt`
- If `resources\banner.default.txt` is missing, a minimal fallback banner is created.
- Run from the project root so the `resources` path resolves.
- `run-loop` can be stopped with Ctrl+C.
