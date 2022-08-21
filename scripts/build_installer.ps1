param(
  [Parameter(HelpMessage="The Qt version to bundle")]
  $QtVersion = 6
)

$ErrorActionPreference = "Stop"

function DownloadLanguageFile($languageFile, $innoPath) {
  $url = 'https://raw.githubusercontent.com/jrsoftware/issrc/is-6_0_5/Files/Languages/Unofficial/' + $languageFile
  $installPath = $innoPath + '\Languages\' + $languageFile

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
}

$innoInstallPath = 'C:\Program Files (x86)\Inno Setup 6'
# Unofficial language files to download and install.
$unofficialLanguageFiles = @(
  'Korean.isl',
  'Swedish.isl',
  'ChineseSimplified.isl'
)

# Install some unofficial translation files for Inno Setup.
foreach ($languageFile in $unofficialLanguageFiles) {
  DownloadLanguageFile $languageFile $innoInstallPath
}

$env:PATH += ';' + $innoInstallPath

Write-Output "Building installer for LOOT with Qt version $QtVersion"
iscc "-DQtVersion=$QtVersion" scripts\installer.iss

if ($LastExitCode -ne 0) {
  throw 'Failed to build the LOOT installer'
}
