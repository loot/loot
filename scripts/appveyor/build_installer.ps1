function DownloadLanguageFile($languageFile, $innoPath) {
  $url = 'https://raw.github.com/jrsoftware/issrc/master/Files/Languages/Unofficial/' + $languageFile
  $installPath = $innoPath + '\Languages\' + $languageFile

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
}

$innoInstallPath = 'C:\Program Files (x86)\Inno Setup 5'

if ($env:APPVEYOR_REPO_TAG -eq 'true') {
  if (!(Test-Path $innoInstallPath)) {
    choco install -y InnoSetup

    # Install the Inno Download Plugin
    $url = 'https://bitbucket.org/mitrich_k/inno-download-plugin/downloads/idpsetup-1.5.1.exe'
    (New-Object System.Net.WebClient).DownloadFile($url, (pwd).path + '/idpsetup-1.5.1.exe')
    ./idpsetup-1.5.1.exe /verysilent

    # Also install the unofficial Korean and Simplified Chinese translation
    # files for Inno Setup.
    DownloadLanguageFile '\Korean.isl'  $innoInstallPath
    DownloadLanguageFile '\ChineseSimplified.isl'  $innoInstallPath
  }

  $env:PATH += ';' + $innoInstallPath

  iscc scripts\installer.iss
}
