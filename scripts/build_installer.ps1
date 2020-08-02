$ErrorActionPreference = "Stop"

function DownloadLanguageFile($languageFile, $innoPath) {
  $url = 'https://raw.githubusercontent.com/jrsoftware/issrc/is-6_0_5/Files/Languages/Unofficial/' + $languageFile
  $installPath = $innoPath + '\Languages\' + $languageFile

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
}

$innoInstallPath = 'C:\Program Files (x86)\Inno Setup 6'

# Install the Inno Download Plugin
$url = 'https://bitbucket.org/mitrich_k/inno-download-plugin/downloads/idpsetup-1.5.1.exe'
(New-Object System.Net.WebClient).DownloadFile($url, (Get-Location).path + '/idpsetup-1.5.1.exe')

Start-Process ./idpsetup-1.5.1.exe /verysilent -Wait
if (!$?) {
  throw 'Failed to install the Inno Download Plugin'
}

# Also install the unofficial Korean, Simplified Chinese and Swedish
# translation files for Inno Setup.
DownloadLanguageFile '\Korean.isl'  $innoInstallPath
DownloadLanguageFile '\ChineseSimplified.isl'  $innoInstallPath
DownloadLanguageFile '\Swedish.isl'  $innoInstallPath

$env:PATH += ';' + $innoInstallPath

iscc scripts\installer.iss
if ($LastExitCode -ne 0) {
  throw 'Failed to build the LOOT installer'
}
