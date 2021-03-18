$ErrorActionPreference = "Stop"

function DownloadLanguageFile($languageFile, $innoPath) {
  $url = 'https://raw.githubusercontent.com/jrsoftware/issrc/is-6_0_5/Files/Languages/Unofficial/' + $languageFile
  $installPath = $innoPath + '\Languages\' + $languageFile

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
}

$innoInstallPath = 'C:\Program Files (x86)\Inno Setup 6'

# Install the unofficial Bulgarian, Korean, Swedish and Simplified Chinese
# translation files for Inno Setup.
DownloadLanguageFile '\Bulgarian.isl'  $innoInstallPath
DownloadLanguageFile '\Korean.isl'  $innoInstallPath
DownloadLanguageFile '\Swedish.isl'  $innoInstallPath
DownloadLanguageFile '\ChineseSimplified.isl'  $innoInstallPath

$env:PATH += ';' + $innoInstallPath

iscc scripts\installer.iss
if ($LastExitCode -ne 0) {
  throw 'Failed to build the LOOT installer'
}
