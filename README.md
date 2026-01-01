# LiveBanner

LiveBanner is a Windows console banner manager and runner. The manager lets you edit or remove the banner, and the runner prints the banner with a simple RGB animation while keeping the cursor usable.

## Download
Option A: Git clone
```
git clone <REPO_URL>
cd LiveBanner
```

Option B: ZIP download
1. Download the ZIP from the repository.
2. Extract it to a folder such as `C:\LiveBanner`.
3. Open a terminal in that folder.

## Build
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
.\LiveBanner.exe
```

Runner (one-shot):
```
.\LiveBanner.exe run
```

Runner (loop):
```
.\LiveBanner.exe run-loop
```

## Notes
- Banner file: `%APPDATA%\LiveBanner\banner.txt`
- If `resources\banner.default.txt` is missing, a minimal fallback banner is created.
- Run from the project root so the `resources` path resolves.
- `run-loop` can be stopped with Ctrl+C.
