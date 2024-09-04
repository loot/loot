$ErrorActionPreference = "Stop"

$masterlistBranch = "v0.21"

function DownloadLanguageFile($languageFile, $parentPath) {
  New-Item -ItemType "directory" -Force $parentPath

  $url = 'https://raw.githubusercontent.com/jrsoftware/issrc/is-6_0_5/Files/Languages/Unofficial/' + $languageFile
  $installPath = Join-Path -Path $parentPath -ChildPath $languageFile

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
  $installDir = Join-Path -Path '.\build\masterlists\' -ChildPath $lootGameFolder
  New-Item -ItemType "directory" -Force $installDir

  $url = "https://raw.githubusercontent.com/loot/$repo/$masterlistBranch/$basename.yaml"
  $installPath = Join-Path -Path $installDir -ChildPath "$basename.yaml"

  (New-Object System.Net.WebClient).DownloadFile($url, $installPath)
  WriteDownloadMetadata $installPath
}

function DownloadMasterlist($repo, $lootGameFolder) {
  DownloadMetadataFile $repo $lootGameFolder 'masterlist'
}

$languagesParentPath = Join-Path -Path 'build' -ChildPath 'inno'
# Unofficial language files to download and install.
$unofficialLanguageFiles = @(
  'Korean.isl',
  'Swedish.isl',
  'ChineseSimplified.isl'
)

# Install some unofficial translation files for Inno Setup.
foreach ($languageFile in $unofficialLanguageFiles) {
  DownloadLanguageFile $languageFile $languagesParentPath
}

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
DownloadMasterlist 'starfield' 'Starfield'
