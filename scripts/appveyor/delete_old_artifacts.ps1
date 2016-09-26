function Get-HashFromBintrayVersion($version) {
  return $version.split('-')[2].split('_')[0].substring(1)
}

function Get-BranchFromBintrayVersion($version) {
  return $version.split('-')[2].split('_-', 2)[1]
}

function Get-AuthorizationHeaderValue($username, $password) {
  $credentials = "$($username):$($password)"
  return 'Basic ' + [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($credentials))
}

$currentBranch = $env:APPVEYOR_REPO_BRANCH
$bintrayToken = $env:bintray_token
$bintrayRepoUrl = 'https://api.bintray.com/packages/wrinklyninja/loot'
$bintrayHeaders = @{ Authorization = (Get-AuthorizationHeaderValue 'wrinklyninja' $bintrayToken) }
$packages = @(
  'LOOT'
  'LOOT_API'
  'metadata-validator'
)

if ($currentBranch -ne "dev") {
  exit
}

$currentCommitId = [string](git rev-parse --short HEAD)

foreach($package in $packages) {
  $bintrayPackageUrl = "$bintrayRepoUrl/$package"
  $versions = (Invoke-RestMethod -Uri $bintrayPackageUrl).versions


  $matchingBranches = @()
  foreach ($version in $versions) {
    if ($currentCommitId -eq (Get-HashFromBintrayVersion $version)) {
      $versionBranch = Get-BranchFromBintrayVersion $version
      if ($versionBranch -ne "dev") {
        $matchingBranches += $versionBranch
      }
    }
  }

  foreach ($version in $versions) {
    $versionBranch = Get-BranchFromBintrayVersion $version

    foreach ($matchingBranch in $matchingBranches) {
      if ($versionBranch -eq $matchingBranch) {
        Invoke-RestMethod -Method Delete -Uri "$bintrayPackageUrl/versions/$version" -Headers $bintrayHeaders
      }
    }
  }
}
