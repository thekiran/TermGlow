[Setup]
AppName=LiveBanner
AppVersion=1.0.0
DefaultDirName={pf}\LiveBanner
DefaultGroupName=LiveBanner
OutputDir=dist
OutputBaseFilename=LiveBannerSetup
Compression=lzma
SolidCompression=yes

[Files]
Source: "LiveBanner.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "resources\*"; DestDir: "{app}\resources"; Flags: recursesubdirs createallsubdirs
Source: "README.md"; DestDir: "{app}"; Flags: isreadme

[Icons]
Name: "{group}\LiveBanner"; Filename: "{app}\LiveBanner.exe"

[Run]
Filename: "{app}\LiveBanner.exe"; Parameters: "install"; StatusMsg: "Kurulum ayarlari yapiliyor..."; Flags: runhidden waituntilterminated
