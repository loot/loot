
$innoInstallPath = 'C:\Program Files (x86)\Inno Setup 5'

if ($env:APPVEYOR_REPO_TAG) {
  if (!(Test-Path $innoInstallPath)) {
    choco install -y InnoSetup

    # Also install the unofficial Korean and Simplified Chinese translation
    # files for Inno Setup.
    $languagesPath = $innoInstallPath + '\Languages'
    $koreanUrl = 'https://raw.github.com/jrsoftware/issrc/master/Files/Languages/Unofficial/Korean.isl'
    $chineseUrl = 'https://raw.github.com/jrsoftware/issrc/master/Files/Languages/Unofficial/ChineseSimplified.isl'

    (New-Object System.Net.WebClient).DownloadFile($koreanUrl, $languagesPath + '\Korean.isl')
    (New-Object System.Net.WebClient).DownloadFile($chineseUrl, $languagesPath + '\ChineseSimplified.isl')
  }

  $env:PATH += ';' + $innoInstallPath

  iscc scripts\installer.iss
}
