;BOSS NSIS Installer Script

;--------------------------------
;Include NSIS files.

	!include "MUI2.nsh"
	!include "LogicLib.nsh"
	!include "nsDialogs.nsh"
    !include "TextReplace.nsh"  ;http://nsis.sourceforge.net/TextReplace_plugin

;--------------------------------
;General

	;Name, file and version info for installer.
	Name "BOSS v3.0.0"
	OutFile "BOSS Installer.exe"
	VIProductVersion 3.0.0.0

	;Request application privileges for Windows Vista/7
	RequestExecutionLevel admin
	
	;Icon for installer\uninstaller
	!define MUI_ICON "..\resources\icon.ico"
	!define MUI_UNICON "..\resources\icon.ico"
	
	; This causes an "are you sure?" message to be displayed if you try to quit the installer or uninstaller.
	!define MUI_ABORTWARNING
	!define MUI_UNABORTWARNING
	
	;Checks that the installer's CRC is correct (means we can remove installer CRC checking from BOSS).
	CRCCheck force
	
	;The SOLID lzma compressor gives the best compression ratio.
	SetCompressor /SOLID lzma

;--------------------------------
;Interface Settings

	

;--------------------------------
;Pages

	!define MUI_CUSTOMFUNCTION_GUIINIT onGUIInit

	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES
	!define MUI_FINISHPAGE_NOAUTOCLOSE
	!define MUI_FINISHPAGE_RUN "$INSTDIR\BOSS.exe"
	!define MUI_FINISHPAGE_RUN_TEXT "$(Text_Run)"
	!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Docs\BOSS Readme.html"
	!define MUI_FINISHPAGE_SHOWREADME_TEXT "$(Text_ShowReadme)"
	!insertmacro MUI_PAGE_FINISH

	!insertmacro MUI_UNPAGE_WELCOME
	!insertmacro MUI_UNPAGE_COMPONENTS
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages

	!insertmacro MUI_LANGUAGE "English"
	!insertmacro MUI_LANGUAGE "Russian"
	!insertmacro MUI_LANGUAGE "German"
	!insertmacro MUI_LANGUAGE "Spanish"
	!insertmacro MUI_LANGUAGE "SimpChinese"
	!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;English Strings

	VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© 2009-2013 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installer for BOSS 3.0.0"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "3.0.0"

	LangString TEXT_MESSAGEBOX ${LANG_ENGLISH} "BOSS is already installed, and must be uninstalled before continuing. $\n$\nClick `OK` to remove the previous version or `Cancel` to cancel this upgrade."
	LangString TEXT_RUN ${LANG_ENGLISH} "Run BOSS"
	LangString TEXT_SHOWREADME ${LANG_ENGLISH} "View Readme"
	LangString TEXT_MAIN ${LANG_ENGLISH} "All BOSS's files, minus userlists and settings file."
	LangString TEXT_USERFILES ${LANG_ENGLISH} "BOSS's userlist files and settings file."

;--------------------------------
;Russian (Русский) Strings

	VIAddVersionKey /LANG=${LANG_RUSSIAN} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_RUSSIAN} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_RUSSIAN} "LegalCopyright" "© 2009-2013 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_RUSSIAN} "FileDescription" "Установщик для BOSS 3.0.0"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "3.0.0"

	LangString TEXT_MESSAGEBOX ${LANG_RUSSIAN} "BOSS уже установлен и должен быть удален перед продолжением. $\n$\nНажмите `OK` для удаления предыдущей версии или `Отмена` для отмены обновления."
	LangString TEXT_RUN ${LANG_RUSSIAN} "Запустить BOSS"
	LangString TEXT_SHOWREADME ${LANG_RUSSIAN} "Смотреть BOSS-Readme"
	LangString TEXT_MAIN ${LANG_RUSSIAN} "Все файлы BOSS, кроме пользовательских списков и BOSS.ini"
	LangString TEXT_USERFILES ${LANG_RUSSIAN} "Файлы пользовательских списков и BOSS.ini."
 
;--------------------------------
;German (Deutsch) Strings

	VIAddVersionKey /LANG=${LANG_GERMAN} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_GERMAN} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_GERMAN} "LegalCopyright" "© 2009-2013 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_GERMAN} "FileDescription" "Installer für BOSS 3.0.0"
	VIAddVersionKey /LANG=${LANG_GERMAN} "FileVersion" "3.0.0"

	LangString TEXT_MESSAGEBOX ${LANG_GERMAN} "BOSS ist bereits installiert und muss deinstalliert werden, bevor fortgefahren wird. $\n$\nKlicke auf `Ok` um die vorherige Version zu entfernen oder auf `Abbrechen` um das Upgrade abzubrechen."
	LangString TEXT_RUN ${LANG_GERMAN} "BOSS starten"
	LangString TEXT_SHOWREADME ${LANG_GERMAN} "Readme lesen"
	LangString TEXT_MAIN ${LANG_GERMAN} "Alle Dateien von BOSS ohne die Benutzerlisten und die BOSS.ini."
	LangString TEXT_USERFILES ${LANG_GERMAN} "Benutzerliste von BOSS und die BOSS.ini-Datei."
	
;--------------------------------
;Spanish (castellano) Strings

	VIAddVersionKey /LANG=${LANG_SPANISH} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_SPANISH} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_SPANISH} "LegalCopyright" "© 2009-2013 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_SPANISH} "FileDescription" "El instalador para BOSS 3.0.0"
	VIAddVersionKey /LANG=${LANG_SPANISH} "FileVersion" "3.0.0"

	LangString TEXT_MESSAGEBOX ${LANG_SPANISH} "BOSS está instalado, y debe ser desinstalado antes de continuar. $\n$\nPresione `OK` para eliminar la versión anterior o `Cancel` para cancelar la actualización."
	LangString TEXT_RUN ${LANG_SPANISH} "Ejecutar BOSS"
	LangString TEXT_SHOWREADME ${LANG_SPANISH} "Ver Léame"
	LangString TEXT_MAIN ${LANG_SPANISH} "Todos los archivos de BOSS, menos BOSS.ini y listas de usuarios."
	LangString TEXT_USERFILES ${LANG_SPANISH} "BOSS.ini y listas de usuarios."
 
;--------------------------------
;Simplified Chinese (简体中文) Strings

VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "ProductName" "BOSS"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "CompanyName" "BOSS Development Team"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "LegalCopyright" "© 2009-2013 BOSS Development Team"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileDescription" "BOSS 3.0.0安装包"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileVersion" "3.0.0"

LangString TEXT_MESSAGEBOX ${LANG_SIMPCHINESE} "检测到旧版BOSS，您需要先卸载旧版才能安装新版。$\n$\n单击“确定”卸载旧版本或者“取消”取消更新。"
LangString TEXT_RUN ${LANG_SIMPCHINESE} "运行BOSS"
LangString TEXT_SHOWREADME ${LANG_SIMPCHINESE} "查看说明"
LangString TEXT_MAIN ${LANG_SIMPCHINESE} "所有BOSS文件（除userlist和BOSS.ini）"
LangString TEXT_USERFILES ${LANG_SIMPCHINESE} "BOSS的userlist和BOSS.ini文件。"
	
;--------------------------------
;Variables

	Var OB_Path
	Var NE_Path
	Var SK_Path
	Var FO_Path
	Var NV_Path
	Var Empty ;An empty string.
	Var InstallPath ;Path to existing BOSS install.

;--------------------------------
;Initialisations

	Function .onInit
	
		!insertmacro MUI_LANGDLL_DISPLAY

		StrCpy $Empty ""

		; Look for games, setting their paths if found.
		ReadRegStr $OB_Path HKLM "Software\Bethesda Softworks\Oblivion" "Installed Path"
		${If} $OB_Path == $Empty ;Try 64 bit path.
			ReadRegStr $OB_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Oblivion" "Installed Path"
		${EndIf}
		ReadRegStr $NE_Path HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Nehrim - At Fate's Edge_is1" "InstallLocation" ;No 64 bit path.
		ReadRegStr $SK_Path HKLM "Software\Bethesda Softworks\Skyrim" "Installed Path"
		${If} $SK_Path == $Empty ;Try 64 bit path.
			ReadRegStr $SK_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Skyrim" "Installed Path"
		${EndIf}
		ReadRegStr $FO_Path HKLM "Software\Bethesda Softworks\Fallout3" "Installed Path"
		${If} $FO_Path == $Empty ;Try 64 bit path.
			ReadRegStr $FO_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Fallout3" "Installed Path"
		${EndIf}
		ReadRegStr $NV_Path HKLM "Software\Bethesda Softworks\FalloutNV" "Installed Path"
		${If} $NV_Path == $Empty ;Try 64 bit path.
			ReadRegStr $NV_Path HKLM "Software\Wow6432Node\Bethesda Softworks\FalloutNV" "Installed Path"
		${EndIf}
		StrCpy $INSTDIR "C:\BOSS"
		
	FunctionEnd
	
	Function onGUIInit
		; Have to do this now as language isn't actually set until 
		; First check to see if BOSS is already installed via installer, and launch the existing uninstaller if so.
		IfFileExists "$COMMONFILES\BOSS\uninstall.exe" 0 +8
			MessageBox MB_OKCANCEL|MB_ICONQUESTION "$(Text_MessageBox)" IDOK oldCont IDCANCEL oldCancel
			oldCancel:
				Quit
			oldCont:
				ExecWait '$COMMONFILES\BOSS\uninstall.exe _?=$COMMONFILES\BOSS' ;Run the uninstaller in its folder and wait until it's done.
				Delete "$COMMONFILES\BOSS\uninstall.exe"
				RMDir "$COMMONFILES\BOSS"

		;That was the old uninstaller location, now see if the current version is already installed.
		ReadRegStr $InstallPath HKLM "Software\BOSS" "Installed Path"
		${If} $InstallPath == $Empty ;Try 64 bit path.
			ReadRegStr $InstallPath HKLM "Software\Wow6432Node\BOSS" "Installed Path"
		${EndIf}
		${If} $InstallPath != $Empty
			StrCpy $INSTDIR $InstallPath  ;Set the default install path to the previous install's path.
			IfFileExists "$InstallPath\Uninstall.exe" 0 +8
				MessageBox MB_OKCANCEL|MB_ICONQUESTION "$(Text_MessageBox)" IDOK cont IDCANCEL cancel
				cancel:
					Quit
				cont:
					ExecWait '$InstallPath\Uninstall.exe _?=$InstallPath' ;Run the uninstaller in its folder and wait until it's done.
					Delete "$InstallPath\Uninstall.exe"
					RMDir "$InstallPath"
		${EndIf}
	FunctionEnd

	Function un.onInit

		!insertmacro MUI_LANGDLL_DISPLAY
		
	FunctionEnd

;--------------------------------
;Installer Sections

	Section "Installer Section"
		
		;Silently move and remove files from past BOSS installs.
		 ${If} $OB_Path != $Empty
;			IfFileExists "$OB_Path\Data\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Oblivion"
;				Rename "$OB_Path\Data\BOSS\userlist.txt" "$INSTDIR\Oblivion\userlist.txt"
;			IfFileExists "$OB_Path\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Oblivion"
;				Rename "$OB_Path\BOSS\userlist.txt" "$INSTDIR\Oblivion\userlist.txt"
			Delete "$OB_Path\Data\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
			Delete "$OB_Path\Data\modlist.*"
			Delete "$OB_Path\Data\masterlist.txt"
			Delete "$OB_Path\Data\BOSS\modlist.*"
			Delete "$OB_Path\Data\BOSS\masterlist.txt"
			Delete "$OB_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
			RMDir  "$OB_Path\Data\BOSS"
		${EndIf}
		${If} $NE_Path != $Empty
;			IfFileExists "$NE_Path\Data\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Nehrim"
;				Rename "$NE_Path\Data\BOSS\userlist.txt" "$INSTDIR\Nehrim\userlist.txt"
;			IfFileExists "$NE_Path\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Nehrim"
;				Rename "$NE_Path\BOSS\userlist.txt" "$INSTDIR\Nehrim\userlist.txt"
			Delete "$NE_Path\Data\BOSS*"
			Delete "$NE_Path\Data\modlist.*"
			Delete "$NE_Path\Data\masterlist.txt"
			Delete "$NE_Path\Data\BOSS\modlist.*"
			Delete "$NE_Path\Data\BOSS\masterlist.txt"
			Delete "$NE_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
			RMDir  "$NE_Path\Data\BOSS"
		${EndIf}
		${If} $SK_Path != $Empty
;			IfFileExists "$SK_Path\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Skyrim"
;				Rename "$SK_Path\BOSS\userlist.txt" "$INSTDIR\Skyrim\userlist.txt"
			Delete "$SK_Path\Data\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
			Delete "$SK_Path\Data\modlist.*"
			Delete "$SK_Path\Data\masterlist.txt"
			Delete "$SK_Path\Data\BOSS\modlist.*"
			Delete "$SK_Path\Data\BOSS\masterlist.txt"
			Delete "$SK_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
			RMDir  "$SK_Path\Data\BOSS"
		${EndIf}
		${If} $FO_Path != $Empty
;			IfFileExists "$FO_Path\Data\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Fallout 3"
;				Rename "$FO_Path\Data\BOSS\userlist.txt" "$INSTDIR\Fallout 3\userlist.txt"
;			IfFileExists "$FO_Path\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Fallout 3"
;				Rename "$FO_Path\BOSS\userlist.txt" "$INSTDIR\Fallout 3\userlist.txt"
			Delete "$FO_Path\Data\BOSS*"
			Delete "$FO_Path\Data\modlist.*"
			Delete "$FO_Path\Data\masterlist.txt"
			Delete "$FO_Path\Data\BOSS\modlist.*"
			Delete "$FO_Path\Data\BOSS\masterlist.txt"
			Delete "$FO_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
			RMDir  "$FO_Path\Data\BOSS"
		${EndIf}
		${If} $NV_Path != $Empty
;			IfFileExists "$NV_Path\Data\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Fallout New Vegas"
;				Rename "$NV_Path\Data\BOSS\userlist.txt" "$INSTDIR\Fallout New Vegas\userlist.txt"
;			IfFileExists "$NV_Path\BOSS\userlist.txt" 0 +3
;				CreateDirectory "$INSTDIR\Fallout New Vegas"
;				Rename "$NV_Path\BOSS\userlist.txt" "$INSTDIR\Fallout New Vegas\userlist.txt"
			Delete "$NV_Path\Data\BOSS*"
			Delete "$NV_Path\Data\modlist.*"
			Delete "$NV_Path\Data\masterlist.txt"
			Delete "$NV_Path\Data\BOSS\modlist.*"
			Delete "$NV_Path\Data\BOSS\masterlist.txt"
			Delete "$NV_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
			RMDir  "$NV_Path\Data\BOSS"
		${EndIf}

        ;Delete BOSS.ini and BOSS.ini.old if they exist.
        Delete "BOSS.ini*"
			
		;Install new BOSS settings.yaml.
		SetOutPath "$LOCALAPPDATA\BOSS"
		File "..\resources\settings.yaml"
		
		;Write language setting to settings.yaml.
		StrCmp $LANGUAGE ${LANG_RUSSIAN} 0 +2
        ${textreplace::ReplaceInFile} "$LOCALAPPDATA\BOSS\settings.yaml" "$LOCALAPPDATA\BOSS\settings.yaml" "eng" "rus" "/S=1 /C=0" 1
		StrCmp $LANGUAGE ${LANG_GERMAN} 0 +2
        ${textreplace::ReplaceInFile} "$LOCALAPPDATA\BOSS\settings.yaml" "$LOCALAPPDATA\BOSS\settings.yaml" "eng" "deu" "/S=1 /C=0" 1
		StrCmp $LANGUAGE ${LANG_SPANISH} 0 +2
        ${textreplace::ReplaceInFile} "$LOCALAPPDATA\BOSS\settings.yaml" "$LOCALAPPDATA\BOSS\settings.yaml" "eng" "spa" "/S=1 /C=0" 1
		StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 +2
        ${textreplace::ReplaceInFile} "$LOCALAPPDATA\BOSS\settings.yaml" "$LOCALAPPDATA\BOSS\settings.yaml" "eng" "cmn" "/S=1 /C=0" 1
		
		;Install executable.
		SetOutPath "$INSTDIR"
		File "..\build\BOSS.exe"
			  
		;Now install API DLLs.
		SetOutPath "$INSTDIR\API"
		File "..\build\boss32.dll"
		File "..\build\boss64.dll"
		
		;Now install readme files.
		SetOutPath "$INSTDIR\Docs"
		File "..\docs\BOSS API Readme.html"
		File "..\docs\BOSS Metadata Syntax.html"
		File "..\docs\BOSS Readme.html"
		File "..\docs\Licenses.txt"
		
		;Now install readme images.
		SetOutPath "$INSTDIR\Docs\images"
		File "..\docs\images\editor.png"
		File "..\docs\images\main.png"
		File "..\docs\images\settings.png"
		File "..\docs\images\viewer-1.png"
		File "..\docs\images\viewer-2.png"
  
		;Now install language files.
		SetOutPath "$INSTDIR\resources\l10n\ru\LC_MESSAGES"
		File "..\resources\l10n\ru\LC_MESSAGES\messages.mo"
		File "..\resources\l10n\ru\LC_MESSAGES\wxstd.mo"
		SetOutPath "$INSTDIR\resources\l10n\es\LC_MESSAGES"
		File "..\resources\l10n\es\LC_MESSAGES\wxstd.mo"
		File "..\resources\l10n\es\LC_MESSAGES\messages.mo"
		SetOutPath "$INSTDIR\resources\l10n\de\LC_MESSAGES"
		File "..\resources\l10n\de\LC_MESSAGES\wxstd.mo"
		SetOutPath "$INSTDIR\resources\l10n\zh\LC_MESSAGES"
		File "..\resources\l10n\zh\LC_MESSAGES\messages.mo"
		File "..\resources\l10n\zh\LC_MESSAGES\wxstd.mo"
		
		;Add Start Menu shortcuts. Set out path back to $INSTDIR otherwise the shortcuts start in the wrong place.
		;Set Shell Var Context to all so that shortcuts are installed for all users, not just admin.
		SetOutPath "$INSTDIR"
		SetShellVarContext all
		CreateDirectory "$SMPROGRAMS\BOSS"
		CreateShortCut "$SMPROGRAMS\BOSS\BOSS.lnk" "$INSTDIR\BOSS.exe"
		CreateShortCut "$SMPROGRAMS\BOSS\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
		CreateShortCut "$SMPROGRAMS\BOSS\Readme.lnk" "$INSTDIR\Docs\BOSS Readme.html"
	  
		
		;Store installation folder in registry key.
		WriteRegStr HKLM "Software\BOSS" "Installed Path" "$INSTDIR"
		;Write registry keys for Windows' uninstaller.
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayName" "BOSS"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "UninstallString" '"$INSTDIR\Uninstall.exe"'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "URLInfoAbout" 'http://better-oblivion-sorting-software.googlecode.com/'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "HelpLink" 'http://better-oblivion-sorting-software.googlecode.com/'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "Publisher" 'BOSS Development Team'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayVersion" '3.0.0'      
		WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoModify" 1
		WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoRepair" 1
		
		;Create uninstaller
		WriteUninstaller "$INSTDIR\Uninstall.exe"

	SectionEnd

;--------------------------------
;Uninstaller Section


	Section "un.BOSS" Main

		;Remove main executables.
		Delete "$INSTDIR\BOSS.exe"
		
		;Remove API DLLs.
		Delete "$INSTDIR\API\boss32.dll"
		Delete "$INSTDIR\API\boss64.dll"
		
		;Remove readme files.
		Delete "$INSTDIR\Docs\BOSS API Readme.html"
		Delete "$INSTDIR\Docs\BOSS Metadata Syntax.html"
		Delete "$INSTDIR\Docs\BOSS Readme.html"
		Delete "$INSTDIR\Docs\Licenses.txt"
		
		;Remove readme images.
		Delete "$INSTDIR\Docs\images\editor.png"
		Delete "$INSTDIR\Docs\images\main.png"
		Delete "$INSTDIR\Docs\images\settings.png"
		Delete "$INSTDIR\Docs\images\viewer-1.png"
		Delete "$INSTDIR\Docs\images\viewer-2.png"
  
		;Remove language files.
		Delete "$INSTDIR\l10n\ru\LC_MESSAGES\messages.mo"
		Delete "$INSTDIR\l10n\ru\LC_MESSAGES\wxstd.mo"
		Delete "$INSTDIR\l10n\es\LC_MESSAGES\messages.mo"
		Delete "$INSTDIR\l10n\es\LC_MESSAGES\wxstd.mo"
		Delete "$INSTDIR\l10n\de\LC_MESSAGES\wxstd.mo"
		Delete "$INSTDIR\l10n\zh\LC_MESSAGES\messages.mo"
		Delete "$INSTDIR\l10n\zh\LC_MESSAGES\wxstd.mo"
		
		;Now we have to remove the files BOSS generates when it runs.
		Delete "$LOCALAPPDATA\BOSS\BOSSDebugLog.txt"
		;Trying to delete a file that doesn't exist doesn't cause an error, so delete all games' files.
        ;This doesn't handle user-defined games.
		Delete "$LOCALAPPDATA\BOSS\Oblivion\report.html"
		Delete "$LOCALAPPDATA\BOSS\Oblivion\masterlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\Nehrim\report.html"
		Delete "$LOCALAPPDATA\BOSS\Nehrim\masterlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\Skyrim\report.html"
		Delete "$LOCALAPPDATA\BOSS\Skyrim\masterlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\Fallout3\report.html"
		Delete "$LOCALAPPDATA\BOSS\Fallout3\masterlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\FalloutNV\report.html"
		Delete "$LOCALAPPDATA\BOSS\FalloutNV\masterlist.yaml"
		
		;Remove subfolders.
		RMDir "$INSTDIR\API"
		RMDir "$INSTDIR\Docs\images"
		RMDir "$INSTDIR\Docs"
		RMDir "$INSTDIR\l10n\ru\LC_MESSAGES"
		RMDir "$INSTDIR\l10n\ru"
		RMDir "$INSTDIR\l10n\es\LC_MESSAGES"
		RMDir "$INSTDIR\l10n\es"
		RMDir "$INSTDIR\l10n\de\LC_MESSAGES"
		RMDir "$INSTDIR\l10n\de"
		RMDir "$INSTDIR\l10n\zh\LC_MESSAGES"
		RMDir "$INSTDIR\l10n\zh"
		RMDir "$INSTDIR\l10n"
		RMDir "$LOCALAPPDATA\BOSS\Oblivion"
		RMDir "$LOCALAPPDATA\BOSS\Nehrim"
		RMDir "$LOCALAPPDATA\BOSS\Skyrim"
		RMDir "$LOCALAPPDATA\BOSS\Fallout3"
		RMDir "$LOCALAPPDATA\BOSS\FalloutNV"
		
		;Remove uninstaller.
		Delete "$INSTDIR\Uninstall.exe"

		;Remove install directory.
		RMDir "$INSTDIR"
        RMDir "$LOCALAPPDATA\BOSS"

		;Delete registry key.
		DeleteRegKey HKLM "Software\BOSS"
		
		;Delete stupid Windows created registry keys:
		DeleteRegKey HKCU "Software\BOSS"
		DeleteRegKey HKLM "Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\BOSS"
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS"
		DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Management\ARPCache\BOSS"
		DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR"
		DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\BOSS.exe"
		DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\Uninstall.exe"
		DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR"
		DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\BOSS.exe"
		DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\Uninstall.exe"
		DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR"
		DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\BOSS.exe"
		DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\Uninstall.exe"
		
		;Delete Start Menu folder.
		SetShellVarContext all
		RMDir /r "$SMPROGRAMS\BOSS"

	SectionEnd

	Section /o "un.User Files" UserFiles
		;The following user files are only removed if set to.
		Delete "$LOCALAPPDATA\BOSS\settings.yaml"
		Delete "$LOCALAPPDATA\BOSS\Oblivion\userlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\Nehrim\userlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\Skyrim\userlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\Fallout3\userlist.yaml"
		Delete "$LOCALAPPDATA\BOSS\FalloutNV\userlist.yaml"
		;Also try removing the folders storing them, in case they are otherwise empty.
		RMDir "$LOCALAPPDATA\BOSS\Oblivion"
		RMDir "$LOCALAPPDATA\BOSS\Nehrim"
		RMDir "$LOCALAPPDATA\BOSS\Skyrim"
		RMDir "$LOCALAPPDATA\BOSS\Fallout3"
		RMDir "$LOCALAPPDATA\BOSS\FalloutNV"
		;Try removing install directory.
        RMDir "$LOCALAPPDATA\BOSS"
	SectionEnd

;--------------------------------
;Languages - Description Strings

	!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
	  !insertmacro MUI_DESCRIPTION_TEXT ${Main} "$(Text_Main)"
	  !insertmacro MUI_DESCRIPTION_TEXT ${UserFiles} "$(Text_UserFiles)"
	!insertmacro MUI_UNFUNCTION_DESCRIPTION_END
	
;--------------------------------
