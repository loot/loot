; LOOT installer Inno Setup script.
; This file must be encoded in UTF-8 WITH a BOM for Unicode text to
; be displayed correctly.

#define MyAppName "LOOT"
#define MyAppVersion "0.23.1"
#define MyAppPublisher "LOOT Team"
#define MyAppURL "https://loot.github.io"
#define MyAppExeName "LOOT.exe"

#define MasterlistBranch "v0.21"

#if FileExists(AddBackslash(SourcePath) + '..\build\inno\Korean.isl')
#define KoreanExists
#endif

#if FileExists(AddBackslash(SourcePath) + '..\build\inno\Swedish.isl')
#define SwedishExists
#endif

#if FileExists(AddBackslash(SourcePath) + '..\build\inno\ChineseSimplified.isl')
#define SimplifiedChineseExists
#endif

#if FileExists(AddBackslash(SourcePath) + '..\build\Release\LOOT.exe')
#define ArtifactsDir "build\Release"
#define LiblootDll "loot.dll"
#else
#define ArtifactsDir "build"
#define LiblootDll "libloot.dll"
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
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
UsePreviousPrivileges=yes

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl,resources\l10n\en\LC_MESSAGES\installer.islu"
Name: "bg"; MessagesFile: "compiler:Languages\Bulgarian.isl,resources\l10n\bg\LC_MESSAGES\installer.islu"
Name: "cs"; MessagesFile: "compiler:Languages\Czech.isl,resources\l10n\cs\LC_MESSAGES\installer.islu"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl,resources\l10n\da\LC_MESSAGES\installer.islu"
Name: "de"; MessagesFile: "compiler:Languages\German.isl,resources\l10n\de\LC_MESSAGES\installer.islu"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl,resources\l10n\es\LC_MESSAGES\installer.islu"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl,resources\l10n\fi\LC_MESSAGES\installer.islu"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl,resources\l10n\fr\LC_MESSAGES\installer.islu"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl,resources\l10n\it\LC_MESSAGES\installer.islu"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl,resources\l10n\ja\LC_MESSAGES\installer.islu"
#ifdef KoreanExists
Name: "ko"; MessagesFile: "build\inno\Korean.isl,resources\l10n\ko\LC_MESSAGES\installer.islu"
#endif
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl,resources\l10n\pl\LC_MESSAGES\installer.islu"
Name: "pt_BR"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl,resources\l10n\pt_BR\LC_MESSAGES\installer.islu"
Name: "pt_PT"; MessagesFile: "compiler:Languages\Portuguese.isl,resources\l10n\pt_PT\LC_MESSAGES\installer.islu"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl,resources\l10n\ru\LC_MESSAGES\installer.islu"
#ifdef SwedishExists
Name: "sv"; MessagesFile: "build\inno\Swedish.isl,resources\l10n\sv\LC_MESSAGES\installer.islu"
#endif
Name: "tr_TR"; MessagesFile: "compiler:Languages\Turkish.isl,resources\l10n\tr_TR\LC_MESSAGES\installer.islu"
Name: "uk_UA"; MessagesFile: "compiler:Languages\Ukrainian.isl,resources\l10n\uk_UA\LC_MESSAGES\installer.islu"
#ifdef SimplifiedChineseExists
Name: "zh_CN"; MessagesFile: "build\inno\ChineseSimplified.isl,resources\l10n\zh_CN\LC_MESSAGES\installer.islu"
#endif

[Tasks]
Name: "masterlists"; Description: "{cm:DownloadMasterlists}"
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#ArtifactsDir}\LOOT.exe"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#ArtifactsDir}\{#LiblootDll}"; \
DestDir: "{app}"; Flags: ignoreversion

#if FileExists(AddBackslash(SourcePath) + '..\' + AddBackslash(ArtifactsDir) + 'Qt6Core.dll')
  
; Common Qt files
Source: "{#ArtifactsDir}\iconengines\*"; \
DestDir: "{app}\iconengines"; Flags: ignoreversion
Source: "{#ArtifactsDir}\imageformats\*"; \
DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "{#ArtifactsDir}\platforms\*"; \
DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#ArtifactsDir}\styles\*"; \
DestDir: "{app}\styles"; Flags: ignoreversion
Source: "{#ArtifactsDir}\translations\*"; \
DestDir: "{app}\translations"; Flags: ignoreversion

; Qt 6 files
Source: "{#ArtifactsDir}\Qt6Core.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#ArtifactsDir}\Qt6Gui.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#ArtifactsDir}\Qt6Network.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#ArtifactsDir}\Qt6Svg.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#ArtifactsDir}\Qt6Widgets.dll"; \
DestDir: "{app}"; Flags: ignoreversion
Source: "{#ArtifactsDir}\networkinformation\*"; \
DestDir: "{app}\networkinformation"; Flags: ignoreversion
Source: "{#ArtifactsDir}\tls\*"; \
DestDir: "{app}\tls"; Flags: ignoreversion

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
Source: "resources\l10n\tr_TR\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\tr_TR\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\uk_UA\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\uk_UA\LC_MESSAGES"; Flags: ignoreversion
Source: "resources\l10n\zh_CN\LC_MESSAGES\loot.mo"; \
DestDir: "{app}\resources\l10n\zh_CN\LC_MESSAGES"; Flags: ignoreversion

; Masterlists
Source: "build\masterlists\prelude\prelude.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\prelude"; Flags: ignoreversion
Source: "build\masterlists\prelude\prelude.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\prelude"; Flags: ignoreversion
Source: "build\masterlists\Morrowind\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Morrowind"; Flags: ignoreversion
Source: "build\masterlists\Morrowind\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Morrowind"; Flags: ignoreversion
Source: "build\masterlists\Oblivion\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Oblivion"; Flags: ignoreversion
Source: "build\masterlists\Oblivion\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Oblivion"; Flags: ignoreversion
Source: "build\masterlists\Nehrim\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Nehrim"; Flags: ignoreversion
Source: "build\masterlists\Nehrim\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Nehrim"; Flags: ignoreversion
Source: "build\masterlists\Skyrim\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Skyrim"; Flags: ignoreversion
Source: "build\masterlists\Skyrim\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Skyrim"; Flags: ignoreversion
Source: "build\masterlists\Enderal\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Enderal"; Flags: ignoreversion
Source: "build\masterlists\Enderal\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Enderal"; Flags: ignoreversion
Source: "build\masterlists\Skyrim Special Edition\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Skyrim Special Edition"; Flags: ignoreversion
Source: "build\masterlists\Skyrim Special Edition\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Skyrim Special Edition"; Flags: ignoreversion
Source: "build\masterlists\Enderal Special Edition\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Enderal Special Edition"; Flags: ignoreversion
Source: "build\masterlists\Enderal Special Edition\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Enderal Special Edition"; Flags: ignoreversion
Source: "build\masterlists\Skyrim VR\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Skyrim VR"; Flags: ignoreversion
Source: "build\masterlists\Skyrim VR\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Skyrim VR"; Flags: ignoreversion
Source: "build\masterlists\Fallout3\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Fallout3"; Flags: ignoreversion
Source: "build\masterlists\Fallout3\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Fallout3"; Flags: ignoreversion
Source: "build\masterlists\FalloutNV\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\FalloutNV"; Flags: ignoreversion
Source: "build\masterlists\FalloutNV\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\FalloutNV"; Flags: ignoreversion
Source: "build\masterlists\Fallout4\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Fallout4"; Flags: ignoreversion
Source: "build\masterlists\Fallout4\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Fallout4"; Flags: ignoreversion
Source: "build\masterlists\Fallout4VR\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Fallout4VR"; Flags: ignoreversion
Source: "build\masterlists\Fallout4VR\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Fallout4VR"; Flags: ignoreversion
Source: "build\masterlists\Starfield\masterlist.yaml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Starfield"; Flags: ignoreversion
Source: "build\masterlists\Starfield\masterlist.yaml.metadata.toml"; \
DestDir: "{localappdata}\{#MyAppName}\games\Starfield"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "{tmp}\vc_redist.2019.x64.exe"; Parameters: "/quiet /norestart"; Flags: skipifdoesntexist; StatusMsg: "{cm:InstallingMSVCRedist}"

[UninstallDelete]
Type: files; Name: "{localappdata}\{#MyAppName}\LOOTDebugLog.txt";

Type: filesandordirs; Name: "{localappdata}\{#MyAppName}\prelude";

Type: dirifempty; Name: "{localappdata}\{#MyAppName}\games";

Type: dirifempty; Name: "{localappdata}\{#MyAppName}";

Type: dirifempty; Name: "{app}\docs\.doctrees\app\usage";
Type: dirifempty; Name: "{app}\docs\.doctrees\app";
Type: dirifempty; Name: "{app}\docs\.doctrees\licenses";
Type: dirifempty; Name: "{app}\docs\.doctrees";
Type: dirifempty; Name: "{app}\docs\_images";
Type: dirifempty; Name: "{app}\docs\_sources\app\usage";
Type: dirifempty; Name: "{app}\docs\_sources\app";
Type: dirifempty; Name: "{app}\docs\_sources\licenses";
Type: dirifempty; Name: "{app}\docs\_sources";
Type: dirifempty; Name: "{app}\docs\_static\css\fonts";
Type: dirifempty; Name: "{app}\docs\_static\css";
Type: dirifempty; Name: "{app}\docs\_static\js";
Type: dirifempty; Name: "{app}\docs\_static";
Type: dirifempty; Name: "{app}\docs\app\usage";
Type: dirifempty; Name: "{app}\docs\app";
Type: dirifempty; Name: "{app}\docs\licenses";
Type: dirifempty; Name: "{app}\docs";
Type: dirifempty; Name: "{app}\iconengines";
Type: dirifempty; Name: "{app}\imageformats";
Type: dirifempty; Name: "{app}\networkinformation";
Type: dirifempty; Name: "{app}\platforms";
Type: dirifempty; Name: "{app}\resources\l10n\bg\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\bg";
Type: dirifempty; Name: "{app}\resources\l10n\cs\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\cs";
Type: dirifempty; Name: "{app}\resources\l10n\da\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\da";
Type: dirifempty; Name: "{app}\resources\l10n\de\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\de";
Type: dirifempty; Name: "{app}\resources\l10n\es\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\es";
Type: dirifempty; Name: "{app}\resources\l10n\fi\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\fi";
Type: dirifempty; Name: "{app}\resources\l10n\fr\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\fr";
Type: dirifempty; Name: "{app}\resources\l10n\it\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\it";
Type: dirifempty; Name: "{app}\resources\l10n\ja\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\ja";
Type: dirifempty; Name: "{app}\resources\l10n\ko\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\ko";
Type: dirifempty; Name: "{app}\resources\l10n\pl\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\pl";
Type: dirifempty; Name: "{app}\resources\l10n\pt_BR\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\pt_BR";
Type: dirifempty; Name: "{app}\resources\l10n\pt_PT\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\pt_PT";
Type: dirifempty; Name: "{app}\resources\l10n\ru\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\ru";
Type: dirifempty; Name: "{app}\resources\l10n\sv\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\sv";
Type: dirifempty; Name: "{app}\resources\l10n\uk_UA\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\uk_UA";
Type: dirifempty; Name: "{app}\resources\l10n\zh_CN\LC_MESSAGES";
Type: dirifempty; Name: "{app}\resources\l10n\zh_CN";
Type: dirifempty; Name: "{app}\resources\l10n";
Type: dirifempty; Name: "{app}\resources";
Type: dirifempty; Name: "{app}\styles";
Type: dirifempty; Name: "{app}\tls";
Type: dirifempty; Name: "{app}\translations";
Type: dirifempty; Name: "{app}";

[Code]
var DownloadPage: TDownloadWizardPage;
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
          SaveStringsToUTF8FileWithoutBOM(File, Lines, False)
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
  if RegQueryStringValue(HKCU, RegKey, 'UninstallString', RegValue) then begin
    Log('Got uninstall string from HKCU registry entry');
    Exec(RemoveQuotes(RegValue), '/VERYSILENT', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end
  else if RegQueryStringValue(HKLM, RegKey, 'UninstallString', RegValue) then begin
    Log('Got uninstall string from HKLM registry entry');
    Exec(RemoveQuotes(RegValue), '/VERYSILENT', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end
  else if RegQueryStringValue(HKLM, 'Software\LOOT', 'Installed Path', RegValue) then begin
    Log('Got uninstall string from the legacy NSIS installer HKLM registry entry');
    Exec(RegValue + '\Uninstall.exe', '/S', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end
  else
    Log('Did not find an uninstaller for any previously-installed version of LOOT.');
end;

function VCRedistNeedsInstall(VersionMajor, VersionMinor, VersionBld: Cardinal): Boolean;
var
  SubKeyName: String;
  IsSuccessful: Boolean;
  IsRuntimeInstalled: Cardinal;
  InstalledVersionMajor: Cardinal;
  InstalledVersionMinor: Cardinal;
  InstalledVersionBld: Cardinal;
begin
  SubKeyName := 'SOFTWARE\Microsoft\VisualStudio\' + IntToStr(VersionMajor) + '.0\VC\Runtimes\x64';

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, 'Installed', IsRuntimeInstalled);

  if (IsSuccessful = False) or (IsRuntimeInstalled <> 1) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 x64 runtime is not installed');
    Result := True;
    exit;
  end;

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, 'Major', InstalledVersionMajor);

  if (IsSuccessful = False) or (InstalledVersionMajor <> VersionMajor) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 x64 runtime major version is not ' + IntToStr(VersionMajor) + ': ' + IntToStr(InstalledVersionMajor));
    Result := True;
    exit;
  end;

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, 'Minor', InstalledVersionMinor);

  if (IsSuccessful = False) or (InstalledVersionMinor < VersionMinor) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 x64 runtime minor version is less than ' + IntToStr(VersionMinor) + ': ' + IntToStr(InstalledVersionMinor));
    Result := True;
    exit;
  end;

  IsSuccessful := RegQueryDwordValue(HKEY_LOCAL_MACHINE, SubKeyName, 'Bld', InstalledVersionBld);

  if (IsSuccessful = False) or (InstalledVersionBld < VersionBld) then begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 x64 runtime build is less than ' + IntToStr(VersionBld) + ': ' + IntToStr(InstalledVersionBld));
    Result := True
  end
  else begin
    Log('MSVC ' + IntToStr(VersionMajor) + '.0 x64 runtime v' + IntToStr(InstalledVersionMajor) + '.' + IntToStr(InstalledVersionMinor) + '.' + IntToStr(InstalledVersionBld) + ' is installed');
    Result := False;
  end;
end;

function CalculateGitBlobHash(const FilePath: String): String;
var Buffer: AnsiString;
var Content: AnsiString;
begin
    if not LoadStringFromFile(FilePath, Content) then begin
      Log(Format('Failed to load file at %s', [FilePath]));
      Result := ''
    end
    else begin
      Buffer := 'blob ' + IntToStr(Length(Content)) + #0 + Content;
      Result := GetSHA1OfString(Buffer);
    end;
end;

procedure WriteDownloadMetadata(const FilePath: String);
var
  MetadataFilePath: String;
  UpdateTimestamp: String;
  BlobSHA1: String;
  Toml: TArrayOfString;
begin
  // Write a <FilePath>.metadata.toml file that contains blob_sha1 and update_timestamp string fields.
  MetadataFilePath := FilePath + '.metadata.toml';
  UpdateTimestamp := GetDateTimeString('yyyy/mm/dd', '-', ':');
  BlobSha1 := CalculateGitBlobHash(FilePath);
  Toml := [
    'blob_sha1 = "' + BlobSha1 + '"',
    'update_timestamp = "' + UpdateTimestamp + '"'
  ];
  if not SaveStringsToUTF8File(MetadataFilePath, Toml, False) then begin
    Log(Format('Failed to write file at %s', [MetadataFilePath]));
  end;
end;

procedure InstallMetadataFile(PathSuffix: String);
var TargetPath: String;
begin
  TargetPath := ExpandConstant('{localappdata}\{#MyAppName}\') + PathSuffix;
  FileCopy(ExpandConstant('{tmp}\') + PathSuffix, TargetPath, False);
  WriteDownloadMetadata(TargetPath);
end;

function OnDownloadProgress(const Url, FileName: String; const Progress, ProgressMax: Int64): Boolean;
begin
  if Progress = ProgressMax then
    Log(Format('Successfully downloaded file to {tmp}: %s', [FileName]));
  Result := True;
end;

// Query user whether their data files should be deleted on uninstall.
procedure CurUninstallStepChanged (CurUninstallStep: TUninstallStep);
var
  FindRec: TFindRec;
  GamePath: String;
  DeleteUserFiles: Boolean;
begin
  // Don't remove user data if the uninstall is silent.
  if UninstallSilent then
    exit;
  if CurUninstallStep = usUninstall then begin
    DeleteUserFiles := MsgBox(CustomMessage('DeleteUserFiles'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES;

    if DeleteUserFiles then begin
      DeleteFile(ExpandConstant('{localappdata}\{#MyAppName}\settings.toml'));
      DelTree(ExpandConstant('{localappdata}\{#MyAppName}\backups\LOOT-backup-*.zip'), False, True, False);
      RemoveDir(ExpandConstant('{localappdata}\{#MyAppName}\backups'));
    end;

    if FindFirst(ExpandConstant('{localappdata}\{#MyAppName}\games\*'), FindRec) then begin
      try
        repeat begin
          if not SameStr(FindRec.Name, '.') and not SameStr(FindRec.Name, '..') and (FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY <> 0) then begin
            GamePath := ExpandConstant('{localappdata}\{#MyAppName}\games\') + FindRec.Name;
            Log(Format('Deleting files from %s', [GamePath]));

            DeleteFile(GamePath + '\masterlist.yaml');
            DeleteFile(GamePath + '\masterlist.yaml.metadata.toml');

            // Although LOOT no longer uses Git repositories, they may be
            // migrated from previous versions of LOOT.
            DelTree(GamePath + '\.git', True, True, True);

            if DeleteUserFiles then begin
              DeleteFile(GamePath + '\userlist.yaml');
              DeleteFile(GamePath + '\group_node_positions.bin');
              DelTree(GamePath + '\loadorder.bak.*', False, True, False);
            end;

            // Try to delete the folder now in case it's empty.
            RemoveDir(GamePath);
          end;
        end
        until not FindNext(FindRec);
      finally
        FindClose(FindRec);
      end;
    end;
  end;
end;

procedure InitializeWizard;
begin
  VC2019RedistNeedsInstall := VCRedistNeedsInstall(14, 15, 26706)

  if VC2019RedistNeedsInstall then begin
    DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), @OnDownloadProgress);
    DownloadPage.Clear;
    DownloadPage.Add('https://aka.ms/vs/16/release/vc_redist.x64.exe', 'vc_redist.2019.x64.exe', '');
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Log(Format('Current page ID: %d', [CurPageID]));
  if (CurPageID = wpSelectTasks) and WizardIsTaskSelected('masterlists') then begin
    if not Assigned(DownloadPage) then begin
      DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), @OnDownloadProgress);
      DownloadPage.Clear;
    end;

    DownloadPage.Add('https://raw.githubusercontent.com/loot/prelude/{#MasterlistBranch}/prelude.yaml', 'prelude\prelude.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/morrowind/{#MasterlistBranch}/masterlist.yaml', 'games\Morrowind\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/oblivion/{#MasterlistBranch}/masterlist.yaml', 'games\Oblivion\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/oblivion/{#MasterlistBranch}/masterlist.yaml', 'games\Nehrim\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/skyrim/{#MasterlistBranch}/masterlist.yaml', 'games\Skyrim\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/enderal/{#MasterlistBranch}/masterlist.yaml', 'games\Enderal\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/skyrimse/{#MasterlistBranch}/masterlist.yaml', 'games\Skyrim Special Edition\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/enderal/{#MasterlistBranch}/masterlist.yaml', 'games\Enderal Special Edition\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/skyrimvr/{#MasterlistBranch}/masterlist.yaml', 'games\Skyrim VR\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/fallout3/{#MasterlistBranch}/masterlist.yaml', 'games\Fallout3\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/falloutnv/{#MasterlistBranch}/masterlist.yaml', 'games\FalloutNV\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/fallout4/{#MasterlistBranch}/masterlist.yaml', 'games\Fallout4\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/fallout4vr/{#MasterlistBranch}/masterlist.yaml', 'games\Fallout4VR\masterlist.yaml', '');
    DownloadPage.Add('https://raw.githubusercontent.com/loot/starfield/{#MasterlistBranch}/masterlist.yaml', 'games\Starfield\masterlist.yaml', '');
  end;

  if Assigned(DownloadPage) and (CurPageID = wpSelectTasks) then begin
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

    if WizardIsTaskSelected('masterlists') then begin
      InstallMetadataFile('prelude\prelude.yaml');
      InstallMetadataFile('games\Morrowind\masterlist.yaml');
      InstallMetadataFile('games\Oblivion\masterlist.yaml');
      InstallMetadataFile('games\Nehrim\masterlist.yaml');
      InstallMetadataFile('games\Skyrim\masterlist.yaml');
      InstallMetadataFile('games\Enderal\masterlist.yaml');
      InstallMetadataFile('games\Skyrim Special Edition\masterlist.yaml');
      InstallMetadataFile('games\Enderal Special Edition\masterlist.yaml');
      InstallMetadataFile('games\Skyrim VR\masterlist.yaml');
      InstallMetadataFile('games\Fallout3\masterlist.yaml');
      InstallMetadataFile('games\FalloutNV\masterlist.yaml');
      InstallMetadataFile('games\Fallout4\masterlist.yaml');
      InstallMetadataFile('games\Fallout4VR\masterlist.yaml');
      InstallMetadataFile('games\Starfield\masterlist.yaml');
    end
  end
end;
