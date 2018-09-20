; LOOT installer Inno Setup script.
; This file must be encoded in UTF-8 WITH a BOM for Unicode text to
; be displayed correctly.

#include <idp.iss>
#include <idplang\finnish.iss>
#include <idplang\french.iss>
#include <idplang\german.iss>
#include <idplang\polish.iss>
#include <idplang\russian.iss>
#include <idplang\spanish.iss>

#define MyAppName "LOOT"
#define MyAppVersion "0.13.6"
#define MyAppPublisher "LOOT Team"
#define MyAppURL "https://loot.github.io"
#define MyAppExeName "LOOT.exe"

#if FileExists(AddBackslash(CompilerPath) + 'Languages\Korean.isl')
#define KoreanExists
#endif

#if FileExists(AddBackslash(CompilerPath) + 'Languages\ChineseSimplified.isl')
#define SimplifiedChineseExists
#endif

#if FileExists(SourcePath + '..\build\32\Release\LOOT.exe')
#define buildir "build\32"
#else
#define buildir "build"
#endif

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{BF634210-A0D4-443F-A657-0DCE38040374}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppCopyright=Copyright (C) 2009-2018 {#MyAppPublisher}
DefaultDirName={pf}\{#MyAppName}
SourceDir=..\
OutputBaseFilename=LOOT Installer
OutputDir=build
SetupIconFile=build\icon\icon.ico
Compression=lzma
SolidCompression=yes
DisableDirPage=no
DisableReadyPage=yes
DisableProgramGroupPage=yes

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "cs"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "pt_BR"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
#ifdef KoreanExists
Name: "ko"; MessagesFile: "compiler:Languages\Korean.isl"
#endif
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"
#ifdef SimplifiedChineseExists
Name: "zh_CN"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
#endif
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#buildir}\Release\LOOT.exe"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\loot_api.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\cef.pak"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\cef_100_percent.pak"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\cef_200_percent.pak"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\d3dcompiler_47.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\devtools_resources.pak"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\icudtl.dat"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\chrome_elf.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\libcef.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\libEGL.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\libGLESv2.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\natives_blob.bin"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\snapshot_blob.bin"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\v8_context_snapshot.bin"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#buildir}\Release\resources\l10n\en-US.pak"; \
DestDir: "{app}\resources\l10n"; Flags: ignoreversion

Source: "{#buildir}\docs\html\*"; \
DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs

Source: "{#buildir}\Release\resources\ui\*"; \
DestDir: "{app}\resources\ui"; Flags: ignoreversion recursesubdirs

Source: "resources\l10n\cs\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\cs\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\da\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\da\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\de\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\de\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\es\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\es\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\fi\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\fi\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\fr\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\fr\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\ko\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\pl\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\pl\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\pt_BR\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\pt_BR\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\ru\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\sv\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\sv\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\zh_CN\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\zh_CN\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\ja\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\ja\LC_MESSAGES"; Flags: ignoreversion

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "{tmp}\vc_redist.x86.exe"; Parameters: "/quiet /norestart"; Flags: skipifdoesntexist

[Registry]
; Store install path for backwards-compatibility with old NSIS install script behaviour.
Root: HKLM; Subkey: "Software\LOOT"; ValueType: string; ValueName: "Installed Path"; ValueData: "{app}"; Flags: deletekey uninsdeletekey

[UninstallDelete]
Type: files; Name: "{localappdata}\{#MyAppName}\";

Type: files; Name: "{localappdata}\{#MyAppName}\Oblivion\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\Skyrim\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\Fallout3\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\FalloutNV\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\Fallout4\masterlist.yaml";

Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\Oblivion\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\Skyrim\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\Fallout3\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\FalloutNV\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\Fallout4\.git";

Type: dirifempty; Name: "{localappdata}\{#MyAppName}\Oblivion";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\Skyrim";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\Fallout3";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\FalloutNV";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\Fallout4";

Type: dirifempty; Name: "{localappdata}\{#MyAppName}";

[CustomMessages]
en.DeleteUserFiles=Do you want to delete your settings and user metadata?
;pt_BR.DeleteUserFiles=
da.DeleteUserFiles=Ønsker du at slette dine indstillinger og bruger metadata?
fi.DeleteUserFiles=Haluatko poistaa asetukset ja käyttäjä metatiedot?
fr.DeleteUserFiles=Voulez-vous supprimer vos paramètres et les métadonnées de l'utilisateur?
de.DeleteUserFiles=Möchten Sie Ihre Einstellungen und Benutzer-Metadaten löschen?
#ifdef KoreanExists
ko.DeleteUserFiles=당신은 당신의 설정과 사용자 메타 데이터를 삭제 하시겠습니까?
#endif
pl.DeleteUserFiles=Czy chcesz usunąć ustawienia i metadane użytkownika?
ru.DeleteUserFiles=Вы хотите удалить ваши настройки и метаданные пользователя?
#ifdef SimplifiedChineseExists
zh_CN.DeleteUserFiles=你想要删除你的设置和用户数据吗？
#endif
es.DeleteUserFiles=¿Quieres borrar sus ajustes y metadatos de usuario?
ja.DeleteUserFiles=設定とユーザーメタデータを削除しますか？
cs.DeleteUserFiles=Vymazat Uživatelské Soubory

[Code]
// Set LOOT's language in settings.toml
procedure SetLootLanguage();
var
  LanguageLine: String;
  File: String;
  SearchLineStart: String;
  Lines: TArrayOfString;
  I: Integer;
begin
  LanguageLine := 'language = "' + ActiveLanguage + '"';
  File := ExpandConstant('{localappdata}\{#MyAppName}\settings.toml');

  if FileExists(File) then begin
    SearchLineStart := 'language = ';

    if LoadStringsFromFile(File, Lines) = True then begin
      for I := 0 to GetArrayLength(Lines) - 1 do begin
        if Copy(Lines[I], 0, Length(SearchLineStart)) = SearchLineStart then begin
          Lines[I] := LanguageLine;
          SaveStringsToUTF8File(File, Lines, False)
          Break;
        end;
      end;
    end;
  end
  else begin
    if ActiveLanguage <> 'en' then
      SaveStringToFile(File, LanguageLine, False);
  end;
end;

// Run a previous install's uninstaller before starting this installation.
procedure RunPreviousVersionUninstaller();
var
  RegKey: String;
  RegValue: String;
  ResultCode: Integer;
begin
  // First try using the Inno Setup installer's uninstall Registry key to get
  // the uninstaller's path. This is necessary instead of just using the
  // backwards-compatible key because the filename of the  uninstaller created
  // by Inno Setup can vary.
  RegKey := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit SetupSetting("AppId")}_is1');
  if RegQueryStringValue(HKLM, RegKey, 'UninstallString', RegValue) then begin
      Exec(RemoveQuotes(RegValue), '/VERYSILENT', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  // Now try using the backwards-compatible Registry key, and run the NSIS
  // uninstaller, which has a fixed filename.
  end
  else begin
    if RegQueryStringValue(HKLM, 'Software\LOOT', 'Installed Path', RegValue) then begin
      Exec(RegValue + '\Uninstall.exe', '/S', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
  end;
end;

function VCRedistNeedsInstall: Boolean;
var
  VersionMajor : Integer;
  VersionMinor: Integer;
  VersionBld: Integer;
  RegKey: String;
  IsRuntimeInstalled: Cardinal;
  InstalledVersionMajor: Cardinal;
  InstalledVersionMinor: Cardinal;
  InstalledVersionBld: Cardinal;
begin
  RegKey := 'Installer\Dependencies\,,x86,14.0,bundle\Dependents\{7e9fae12-5bbf-47fb-b944-09c49e75c061}';

  Result := not RegKeyExists(HKEY_CLASSES_ROOT, RegKey);
end;

// Query user whether their data files should be deleted on uninstall.
procedure CurUninstallStepChanged (CurUninstallStep: TUninstallStep);
begin
  // Don't remove user data if the uninstall is silent.
  if UninstallSilent then
    exit;
  if CurUninstallStep = usUninstall then begin
    if MsgBox(CustomMessage('DeleteUserFiles'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES
    then begin
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\LOOTDebugLog.txt'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\settings.toml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Oblivion\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Skyrim\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Fallout3\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\FalloutNV\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Fallout4\userlist.yaml'));
    end;
  end;
end;

procedure InitializeWizard();
begin
  if VCRedistNeedsInstall then begin
    idpAddFile('https://download.visualstudio.microsoft.com/download/pr/749aa419-f9e4-4578-a417-a43786af205e/d59197078cc425377be301faba7dd87a/vc_redist.x86.exe', ExpandConstant('{tmp}\vc_redist.x86.exe'));

    idpDownloadAfter(wpReady);
  end
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
    RunPreviousVersionUninstaller();

  if CurStep = ssPostInstall then begin
    SetLootLanguage();
    DeleteFile(ExpandConstant('{tmp}\vc_redist.x86.exe'));
  end
end;
