$ErrorActionPreference = "Stop"

$masterlistBranch = "v0.21"

function DownloadLanguageFile($languageFile, $innoPath) {
  $url = 'https://raw.githubusercontent.com/jrsoftware/issrc/is-6_0_5/Files/Languages/Unofficial/' + $languageFile
  $installPath = $innoPath + '\Languages\' + $languageFile

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
}

function GetGitBlobSha1($filePath) {
  $content = [System.IO.File]::ReadAllBytes($filePath)

  $stringAsStream = [System.IO.MemoryStream]::new()

  $writer = [System.IO.BinaryWriter]::new($stringAsStream)
  $writer.write([Text.Encoding]::ASCII.GetBytes('blob ' + $content.Length + "`0"))
  $writer.write($content)
  $writer.Flush()
  $stringAsStream.Position = 0

  return Get-FileHash -InputStream $stringAsStream -Algorithm "SHA1" | Select -ExpandProperty Hash
}

function WriteDownloadMetadata($filePath) {
  $sha1 = GetGitBlobSha1 $filePath
  $date = Get-Date -Format 'yyyy-MM-dd'
  $toml = 'blob_sha1 = "' + $sha1.ToLower() + """`nupdate_timestamp = """ + $date + '"';

  Out-File -FilePath ($filePath + ".metadata.toml") -InputObject $toml
}

function DownloadMetadataFile($repo, $lootGameFolder, $basename) {
  $installDir = '.\build\masterlists\' + $lootGameFolder
  mkdir -Force $installDir

  $url = "https://raw.githubusercontent.com/loot/$repo/$masterlistBranch/$basename.yaml"
  $installPath = $installDir + "\$basename.yaml"

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
  WriteDownloadMetadata $installPath
}

function DownloadMasterlist($repo, $lootGameFolder) {
  DownloadMetadataFile $repo $lootGameFolder 'masterlist'
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

Write-Output 'Downloading masterlists'
DownloadMetadataFile 'prelude' 'prelude' 'prelude'
DownloadMasterlist 'morrowind' 'Morrowind'
DownloadMasterlist 'oblivion' 'Oblivion'
DownloadMasterlist 'oblivion' 'Nehrim'
DownloadMasterlist 'skyrim' 'Skyrim'
DownloadMasterlist 'enderal' 'Enderal'
DownloadMasterlist 'skyrimse' 'Skyrim Special Edition'
DownloadMasterlist 'enderal' 'Enderal Special Edition'
DownloadMasterlist 'skyrimvr' 'Skyrim VR'
DownloadMasterlist 'fallout3' 'Fallout3'
DownloadMasterlist 'falloutnv' 'FalloutNV'
DownloadMasterlist 'fallout4' 'Fallout4'
DownloadMasterlist 'fallout4vr' 'Fallout4VR'

Write-Output "Building installer for LOOT"
iscc scripts\installer.iss

if ($LastExitCode -ne 0) {
  throw 'Failed to build the LOOT installer'
}
