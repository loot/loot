param (
  [Parameter(Mandatory=$True)]
  [string] $url,
  [Parameter(Mandatory=$True)]
  [string] $sha256Hash
)

$ErrorActionPreference = "Stop"

$filename = Split-Path $url -Leaf

Start-BitsTransfer -Source $url -Destination $filename -Dynamic

$hash = Get-FileHash -Algorithm SHA256 $filename

if ($hash.Hash -ne $sha256Hash) {
    throw "Unexpected hash for $($filename): $($hash.Hash)"
}

return $filename
