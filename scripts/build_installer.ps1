$ErrorActionPreference = "Stop"

function DownloadLanguageFile($languageFile, $innoPath) {
  $url = 'https://raw.githubusercontent.com/jrsoftware/issrc/is-6_0_5/Files/Languages/Unofficial/' + $languageFile
  $installPath = $innoPath + '\Languages\' + $languageFile

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
}

$innoInstallPath = 'C:\Program Files (x86)\Inno Setup 6'

# Download the MSVC 2017 redistributable.
$url = 'https://download.visualstudio.microsoft.com/download/pr/749aa419-f9e4-4578-a417-a43786af205e/d59197078cc425377be301faba7dd87a/vc_redist.x86.exe'
(New-Object System.Net.WebClient).DownloadFile($url, (Get-Location).path + '/build/vc_redist.x86.exe')

# Install the unofficial Korean, Simplified Chinese and Swedish translation
# files for Inno Setup.
DownloadLanguageFile '\Korean.isl'  $innoInstallPath
DownloadLanguageFile '\ChineseSimplified.isl'  $innoInstallPath
DownloadLanguageFile '\Swedish.isl'  $innoInstallPath

$env:PATH += ';' + $innoInstallPath

iscc scripts\installer.iss
if ($LastExitCode -ne 0) {
  throw 'Failed to build the LOOT installer'
}
