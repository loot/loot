﻿; LOOT installer Inno Setup script.
; This file must be encoded in UTF-8 WITH a BOM for Unicode text to
; be displayed correctly.

#define MyAppName "LOOT"
#define MyAppVersion "0.18.2"
#define MyAppPublisher "LOOT Team"
#define MyAppURL "https://loot.github.io"
#define MyAppExeName "LOOT.exe"

#if FileExists(AddBackslash(CompilerPath) + 'Languages\Korean.isl')
#define KoreanExists
#endif

#if FileExists(AddBackslash(CompilerPath) + 'Languages\Swedish.isl')
#define SwedishExists
#endif

#if FileExists(AddBackslash(CompilerPath) + 'Languages\ChineseSimplified.isl')
#define SimplifiedChineseExists
#endif

#ifdef MyAppIs64Bit
#define VCRedistArch "x64"
#else
#define VCRedistArch "x86"
#endif

#ifndef QtVersion
#define QtVersion "6"
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
AppCopyright=Copyright (C) 2009 {#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
SourceDir=..\
OutputBaseFilename=LOOT Installer
OutputDir=build
SetupIconFile=build\icon\icon.ico
Compression=lzma
SolidCompression=yes
DisableDirPage=no
DisableReadyPage=yes
DisableProgramGroupPage=yes
WizardStyle=modern

#ifdef MyAppIs64Bit
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#endif

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "bg"; MessagesFile: "compiler:Languages\Bulgarian.isl"
Name: "cs"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
#ifdef KoreanExists
Name: "ko"; MessagesFile: "compiler:Languages\Korean.isl"
#endif
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "pt_BR"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "pt_PT"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"
#ifdef SwedishExists
Name: "sv"; MessagesFile: "compiler:Languages\Swedish.isl"
#endif
Name: "uk_UA"; MessagesFile: "compiler:Languages\Ukrainian.isl"
#ifdef SimplifiedChineseExists
Name: "zh_CN"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
#endif

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "build\Release\LOOT.exe"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\loot.dll"; \
DestDir: "{app}"; Flags: ignoreversion

; Common Qt files
Source: "build\Release\iconengines\*"; \
DestDir: "{app}\iconengines"; Flags: ignoreversion
Source: "build\Release\imageformats\*"; \
DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "build\Release\platforms\*"; \
DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "build\Release\styles\*"; \
DestDir: "{app}\styles"; Flags: ignoreversion
Source: "build\Release\translations\*"; \
DestDir: "{app}\translations"; Flags: ignoreversion

#if QtVersion == "6"
; Qt 6 files
Source: "build\Release\Qt6Core.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt6Gui.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt6Network.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt6Svg.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt6Widgets.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\networkinformation\*"; \
DestDir: "{app}\networkinformation"; Flags: ignoreversion
Source: "build\Release\tls\*"; \
DestDir: "{app}\tls"; Flags: ignoreversion
#elif QtVersion == "5"
; Qt 5 files
Source: "build\Release\Qt5Core.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt5Gui.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt5Network.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt5Svg.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\Qt5Widgets.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\bearer\*"; \
DestDir: "{app}\bearer"; Flags: ignoreversion

#ifdef MyAppIs64Bit
Source: "build\Release\libcrypto-1_1-x64.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\libssl-1_1-x64.dll"; \
DestDir: "{app}"; Flags: ignoreversion
#else
Source: "build\Release\libcrypto-1_1.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\libssl-1_1.dll"; \
DestDir: "{app}"; Flags: ignoreversion
#endif
#endif

Source: "build\docs\html\*"; \
DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs

Source: "resources\l10n\bg\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\bg\LC_MESSAGES"; Flags: ignoreversion
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
Source: "resources\l10n\it\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\it\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\ja\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\ja\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\ko\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\pl\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\pl\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\pt_BR\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\pt_BR\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\pt_PT\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\pt_PT\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\ru\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\sv\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\sv\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\uk_UA\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\uk_UA\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\zh_CN\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\zh_CN\LC_MESSAGES"; Flags: ignoreversion

Source: "{tmp}\vc_redist.{#VCRedistArch}.exe"; DestDir: {tmp}; Flags: deleteafterinstall external skipifsourcedoesntexist

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

#if QtVersion == "5"
Filename: "{tmp}\vc_redist.2010.{#VCRedistArch}.exe"; Parameters: "/q /norestart"; Flags: skipifdoesntexist; StatusMsg: Installing Visual C++ 2010 Redistributable...
#endif

Filename: "{tmp}\vc_redist.2019.{#VCRedistArch}.exe"; Parameters: "/quiet /norestart"; Flags: skipifdoesntexist; StatusMsg: Installing Visual C++ 2019 Redistributable...

[Registry]
; Store install path for backwards-compatibility with old NSIS install script behaviour.
Root: HKLM; Subkey: "Software\LOOT"; ValueType: string; ValueName: "Installed Path"; ValueData: "{app}"; Flags: deletekey uninsdeletekey

[UninstallDelete]
Type: files; Name: "{localappdata}\{#MyAppName}\";

Type: files; Name: "{localappdata}\{#MyAppName}\games\Morrowind\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Oblivion\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Nehrim\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Skyrim\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Enderal\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Skyrim Special Edition\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Enderal Special Edition\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Skyrim VR\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Fallout3\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\FalloutNV\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Fallout4\masterlist.yaml";
Type: files; Name: "{localappdata}\{#MyAppName}\games\Fallout4VR\masterlist.yaml";

; Although LOOT no longer uses Git repositories, they may be migrated from
; previous versions of LOOT.
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Morrowind\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Oblivion\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Nehrim\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Skyrim\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Enderal\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Skyrim Special Edition\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Enderal Special Edition\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Skyrim VR\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Fallout3\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\FalloutNV\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Fallout4\.git";
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\games\Fallout4VR\.git";

Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Morrowind";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Oblivion";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Nehrim";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Skyrim";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Enderal";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Skyrim Special Edition";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Enderal Special Edition";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Skyrim VR";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Fallout3";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\FalloutNV";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Fallout4";
Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games\Fallout4VR";

Type: dirifempty; Name: "{localappdata}\{#MyAppName}";

[CustomMessages]
en.DeleteUserFiles=Do you want to delete your settings and user metadata?
bg.DeleteUserFiles=Искате ли да изтриете Вашите настройки и потребителските метаданни?
cs.DeleteUserFiles=Vymazat Uživatelské Soubory
da.DeleteUserFiles=Ønsker du at slette dine indstillinger og bruger metadata?
de.DeleteUserFiles=Möchten Sie Ihre Einstellungen und Benutzer-Metadaten löschen?
es.DeleteUserFiles=¿Desea borrar sus ajustes y metadatos de usuario?
fi.DeleteUserFiles=Haluatko poistaa asetukset ja käyttäjä metatiedot?
fr.DeleteUserFiles=Voulez-vous supprimer vos paramètres et les métadonnées de l'utilisateur?
it.DeleteUserFiles=Vuoi cancellare le tue impostazioni e i meta-dati utente?
ja.DeleteUserFiles=設定とユーザーメタデータを削除しますか？
#ifdef KoreanExists
ko.DeleteUserFiles=당신은 당신의 설정과 사용자 메타 데이터를 삭제 하시겠습니까?
#endif
pl.DeleteUserFiles=Czy chcesz usunąć ustawienia i metadane użytkownika?
pt_BR.DeleteUserFiles=Você quer deletar suas configurações e dados de usuário?
pt_PT.DeleteUserFiles=Deseja apagar as suas configurações e metadados de utilizador?
ru.DeleteUserFiles=Вы хотите удалить ваши настройки и метаданные пользователя?
;#ifdef SwedishExists
;sv.DeleteUserFiles=
;#endif
uk_UA.DeleteUserFiles=Чи ви хочете видалити ваші налаштування та метадані користувача?
#ifdef SimplifiedChineseExists
zh_CN.DeleteUserFiles=你想要删除你的设置和用户数据吗？
#endif

[Code]
var DownloadPage: TDownloadWizardPage;
var VC2010RedistNeedsInstall: Boolean;
var VC2019RedistNeedsInstall: Boolean;

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

function VCRedistNeedsInstall(VersionMajor, VersionMinor, VersionBld: Cardinal): Boolean;
var
  RuntimesPathComponent: String;
  MajorValueName: String;
  MinorValueName: String;
  SubKeyName: String;
  IsSuccessful: Boolean;
  IsRuntimeInstalled: Cardinal;
  InstalledVersionMajor: Cardinal;
  InstalledVersionMinor: Cardinal;
  InstalledVersionBld: Cardinal;
  Arch: String;
begin
  if VersionMajor = 10 then begin
    RuntimesPathComponent := 'VCRedist';
    MajorValueName := 'MajorVersion';
    MinorValueName := 'MinorVersion';
  end
  else begin
    RuntimesPathComponent := 'Runtimes';
    MajorValueName := 'Major';
    MinorValueName := 'Minor';
  end;

  SubKeyName := 'SOFTWARE\Microsoft\VisualStudio\' + IntToStr(VersionMajor) + '.0\VC\' + RuntimesPathComponent + '\{#VCRedistArch}';

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, 'Installed', IsRuntimeInstalled);

  if (IsSuccessful = False) or (IsRuntimeInstalled <> 1) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 {#VCRedistArch} runtime is not installed');
    Result := True;
    exit;
  end;

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, MajorValueName, InstalledVersionMajor);

  if (IsSuccessful = False) or (InstalledVersionMajor <> VersionMajor) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 {#VCRedistArch} runtime major version is not ' + IntToStr(VersionMajor) + ': ' + IntToStr(InstalledVersionMajor));
    Result := True;
    exit;
  end;

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, MinorValueName, InstalledVersionMinor);

  if (IsSuccessful = False) or (InstalledVersionMinor < VersionMinor) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 {#VCRedistArch} runtime minor version is less than ' + IntToStr(VersionMinor) + ': ' + IntToStr(InstalledVersionMinor));
    Result := True;
    exit;
  end;

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, 'Bld', InstalledVersionBld);

  if (IsSuccessful = False) or (InstalledVersionBld < VersionBld) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 {#VCRedistArch} runtime build is less than ' + IntToStr(VersionBld) + ': ' + IntToStr(InstalledVersionBld));
    Result := True
  end
  else begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 {#VCRedistArch} runtime v' + IntToStr(InstalledVersionMajor) + '.' + IntToStr(InstalledVersionMinor) + '.' + IntToStr(InstalledVersionBld) + ' is installed');
    Result := False;
  end;
end;

function OnDownloadProgress(const Url, FileName: String; const Progress, ProgressMax: Int64): Boolean;
begin
  if Progress = ProgressMax then
    Log(Format('Successfully downloaded file to {tmp}: %s', [FileName]));
  Result := True;
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
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Morrowind\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Oblivion\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Skyrim\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Fallout3\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\FalloutNV\userlist.yaml'));
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\Fallout4\userlist.yaml'));
    end;
  end;
end;

procedure InitializeWizard;
begin
#if QtVersion == "5"
  VC2010RedistNeedsInstall := VCRedistNeedsInstall(10, 0, 40219)
#endif

  VC2019RedistNeedsInstall := VCRedistNeedsInstall(14, 15, 26706)

  if VC2019RedistNeedsInstall or VC2010RedistNeedsInstall then begin
    DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), @OnDownloadProgress);
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
        Log(Format('Current page ID: %d', [CurPageID]));
  if Assigned(DownloadPage) and (CurPageID = wpSelectTasks) then begin
    DownloadPage.Clear;

#if QtVersion == "5"
    if VC2010RedistNeedsInstall then begin
#if VCRedistArch == "x86"
      DownloadPage.Add('https://download.microsoft.com/download/C/6/D/C6D0FD4E-9E53-4897-9B91-836EBA2AACD3/vcredist_x86.exe', 'vc_redist.2010.x86.exe', '');
#else
      DownloadPage.Add('https://download.microsoft.com/download/A/8/0/A80747C3-41BD-45DF-B505-E9710D2744E0/vcredist_x64.exe', 'vc_redist.2010.x64.exe', '');
#endif
    end;
#endif

    if VC2019RedistNeedsInstall then begin
      DownloadPage.Add('https://aka.ms/vs/16/release/vc_redist.{#VCRedistArch}.exe', 'vc_redist.2019.{#VCRedistArch}.exe', '');
    end;

    DownloadPage.Show;
    try
      try
        DownloadPage.Download;
        Result := True;
      except
        SuppressibleMsgBox(AddPeriod(GetExceptionMessage), mbCriticalError, MB_OK, IDOK);
        Result := False;
      end;
    finally
      DownloadPage.Hide;
    end;
  end else
    Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
    RunPreviousVersionUninstaller();

  if CurStep = ssPostInstall then begin
    SetLootLanguage();
  end
end;
