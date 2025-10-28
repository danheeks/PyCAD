; HeeksCAD without  machining
; this installer is the base for all the Heeks2 suite of programs

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{FF4D7ACD-B4B6-49B7-BECF-A7BF5EEC0D21}
AppName=Heeks CAD
AppVerName=Heeks CAD 2.0.1
AppPublisher=https://sites.google.com/view/heeks2/home
AppPublisherURL=https://sites.google.com/view/heeks2/home
AppSupportURL=https://sites.google.com/view/heeks2/home
AppUpdatesURL=https://sites.google.com/view/heeks2/home
DefaultDirName={pf}\HeeksCAD
DefaultGroupName=Heeks CAD
OutputBaseFilename=Heeks CAD 2.0.1
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Dev\PyCAD\HeeksCAD.bat"; DestDir: "{app}\PyCAD"; Flags: ignoreversion; Permissions: users-modify
Source: "C:\Dev\PyCAD\*.py"; DestDir: "{app}\PyCAD"; Flags: ignoreversion; Permissions: users-modify
Source: "C:\Dev\PyCAD\*.pyd"; DestDir: "{app}\PyCAD"; Flags: ignoreversion
Source: "C:\Dev\PyCAD\*.dll"; DestDir: "{app}\PyCAD"; Flags: ignoreversion
Source: "C:\Dev\PyCAD\*.png"; DestDir: "{app}\PyCAD"; Flags: ignoreversion; Permissions: users-modify
Source: "C:\Dev\PyCAD\bitmaps\*.png"; DestDir: "{app}\PyCAD\bitmaps"; Flags: ignoreversion; Permissions: users-modify
Source: "C:\Dev\PyCAD\bitmaps\angle\*.png"; DestDir: "{app}\PyCAD\bitmaps\angle"; Flags: ignoreversion; Permissions: users-modify
Source: "C:\Dev\PyCAD\bitmaps\mirror\*.png"; DestDir: "{app}\PyCAD\bitmaps\mirror"; Flags: ignoreversion; Permissions: users-modify
Source: "C:\Dev\PyCAD\icons\*.png"; DestDir: "{app}\PyCAD\icons"; Flags: ignoreversion; Permissions: users-modify

Source: "C:\Users\Admin\Downloads\Heeks2Dist\python3\*.py"; DestDir: "{app}\python3"; Flags: ignoreversion recursesubdirs; Permissions: users-modify
Source: "C:\Users\Admin\Downloads\Heeks2Dist\python3\*.dll"; DestDir: "{app}\python3"; Flags: ignoreversion recursesubdirs
Source: "C:\Users\Admin\Downloads\Heeks2Dist\python3\*.exe"; DestDir: "{app}\python3"; Flags: ignoreversion recursesubdirs
Source: "C:\Users\Admin\Downloads\Heeks2Dist\python3\*.pyd"; DestDir: "{app}\python3"; Flags: ignoreversion recursesubdirs

;Source: "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.MFC\*"; DestDir: "{app}"; Flags: ignoreversion
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.OPENMP\*"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\*"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Windows\SysWOW64\mfc140u.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Windows\SysWOW64\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Windows\SysWOW64\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Windows\SysWOW64\vcomp140.dll"; DestDir: "{app}"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Heeks CAD"; Filename: "{app}\PyCAD\HeeksCAD.bat"; WorkingDir: "{app}\PyCAD"
Name: "{commondesktop}\Heeks CAD"; Filename: "{app}\PyCAD\HeeksCAD.bat"; WorkingDir: "{app}\PyCAD"; Tasks: desktopicon

