function Get-HashFromBintrayVersion($version) {
  return $version.split('-')[2].split('_')[0].substring(1)
}

function Get-BranchFromBintrayVersion($version) {
  return $version.substring($version.IndexOf('_') + 1)
}

function Get-Branches($versions) {
  $branches = @()
  foreach ($version in $versions) {
    $branches += Get-BranchFromBintrayVersion $version
  }

  return $branches | select -uniq
}

function Is-Merged($commitId) {
  $branches = [array](git branch --contains $commitId)

  return ($branches -contains '  dev') -Or ($branches -contains '* dev')
}

function Get-VersionsForBranch($versions, $branch) {
  $branchVersions = @()
  foreach ($version in $versions) {
    $versionBranch = Get-BranchFromBintrayVersion $version
    if ($versionBranch -eq $branch) {
      $branchVersions += $version
    }
  }
  return $branchVersions
}

function Get-AuthorizationHeaderValue($username, $password) {
  $credentials = "$($username):$($password)"
  return 'Basic ' + [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($credentials))
}

$currentBranch = $env:APPVEYOR_REPO_BRANCH
$bintrayToken = $env:bintray_auth_token
$bintrayRepoUrl = 'https://api.bintray.com/packages/wrinklyninja/loot'
$bintrayHeaders = @{ Authorization = (Get-AuthorizationHeaderValue 'wrinklyninja' $bintrayToken) }
$packages = @(
  'LOOT'
  'LOOT_API'
  'metadata-validator'
)
# The GitHub API's protected branches API is currently in preview, so hardcode
# the branches for now.
$protectedBranches = @(
  'dev'
  'master'
)

foreach($package in $packages) {
  $bintrayPackageUrl = "$bintrayRepoUrl/$package"
  $versions = (Invoke-RestMethod -Uri $bintrayPackageUrl).versions

  $versionsToDelete = @()
  $versionsToKeep = @()

  foreach ($branch in (Get-Branches $versions)) {
    if ($protectedBranches -contains $branch) {
      continue
    }

    $branchVersions = @(Get-VersionsForBranch $versions $branch)
    if (Is-Merged (Get-HashFromBintrayVersion $branchVersions[0])) {
      $versionsToDelete += $branchVersions
    } else {
      $versionsToKeep += $branchVersions[0]
    }
  }

  # Get the versions not marked for deletion or keeping.
  $unevaluatedVersions = @()
  foreach ($version in $versions) {
    if (!($versionsToDelete -contains $version) -And !($versionsToKeep -contains $version)) {
      $unevaluatedVersions += $version
    }
  }

  $firstOldVersionIndex = 40 - $versionsToKeep.length
  if ($unevaluatedVersions.length -gt $firstOldVersionIndex) {
    $versionsToDelete += $unevaluatedVersions[$firstOldVersionIndex..($unevaluatedVersions.length - 1)]
  }

  foreach ($version in $versionsToDelete) {
    Write-Output "Deleting from Bintray: $version"
    Invoke-RestMethod -Method Delete -Uri "$bintrayPackageUrl/versions/$version" -Headers $bintrayHeaders
  }
}
