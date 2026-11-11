#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif

[Setup]
AppId={{6B0E4C1A-3F52-4D8E-9B77-5A1C2D9E7F10}
AppName=LidSleepTray
AppVersion={#AppVersion}
AppPublisher=francknos
DefaultDirName={autopf}\LidSleepTray
DefaultGroupName=LidSleepTray
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=LidSleepTray-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupIconFile=..\assets\setup.ico
UninstallDisplayIcon={app}\uninstall.ico
CloseApplications=yes
RestartApplications=no
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "autostart"; Description: "Lancer au démarrage de Windows"; Flags: unchecked
Name: "desktopicon"; Description: "Créer un raccourci sur le Bureau"; Flags: unchecked

[Files]
Source: "..\dist\LidSleepTray\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion
Source: "..\assets\uninstall.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\LidSleepTray"; Filename: "{app}\LidSleepTray.exe"
Name: "{autodesktop}\LidSleepTray"; Filename: "{app}\LidSleepTray.exe"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "LidSleepTray"; ValueData: """{app}\LidSleepTray.exe"""; Flags: uninsdeletevalue; Tasks: autostart

[Run]
Filename: "{app}\LidSleepTray.exe"; Description: "Lancer LidSleepTray"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{sys}\taskkill.exe"; Parameters: "/F /IM LidSleepTray.exe"; Flags: runhidden; RunOnceId: "KillApp"
