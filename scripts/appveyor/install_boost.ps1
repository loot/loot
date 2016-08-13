Add-Type -AssemblyName System.IO.Compression.FileSystem

$boostUrl = 'https://downloads.sourceforge.net/project/boost/boost/1.61.0/boost_1_61_0.7z?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F1.61.0%2F&ts=1468862599&use_mirror=ncu'
$boostArchive = 'C:\projects\boost_1_61_0.7z'
$boostFolder = 'C:\projects\boost_1_61_0'
$boostLibraries = @(
  'libboost_atomic-vc140-mt-1_61.lib'
  'libboost_chrono-vc140-mt-1_61.lib'
  'libboost_date_time-vc140-mt-1_61.lib'
  'libboost_filesystem-vc140-mt-1_61.lib'
  'libboost_iostreams-vc140-mt-1_61.lib'
  'libboost_locale-vc140-mt-1_61.lib'
  'libboost_log_setup-vc140-mt-1_61.lib'
  'libboost_log-vc140-mt-1_61.lib'
  'libboost_regex-vc140-mt-1_61.lib'
  'libboost_system-vc140-mt-1_61.lib'
  'libboost_thread-vc140-mt-1_61.lib'
)

function Is-LibraryMissing {
  $libFolder = $boostFolder + '\stage\lib'
  foreach($library in $boostLibraries) {
    if (!(Test-Path ($libFolder + '\' + $library))) {
      return $true
    }
  }

  return $false
}

if (Is-LibraryMissing) {
  Write-Output 'Downloading Boost 1.61.0...'

  $start_time = Get-Date
  (New-Object System.Net.WebClient).DownloadFile($boostUrl, $boostArchive)

  Write-Output "Time taken: $((Get-Date).Subtract($start_time).Seconds) second(s)"

  Remove-Item -Recurse -Force $boostFolder

  Write-Output 'Extracting ' + $boostArchive + '...'
  cd
  7z x $boostArchive -o"C:\projects"

  cd $boostFolder
  .\bootstrap.bat
  .\b2 toolset=msvc threadapi=win32 link=static variant=release address-model=32 --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system  --with-iostreams
}
