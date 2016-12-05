function Get-BranchName() {
  $numbers = $env:APPVEYOR_REPO_TAG_NAME.Split('.')

  if ($numbers[0] -eq 0) {
    $numbers = $numbers[0..($numbers.length - 2)]
  } else {
    $numbers = @($numbers[0])
  }

  return "v" + ($numbers -join '.')
}

if ($env:APPVEYOR_REPO_TAG -eq 'true') {
  npm install -g lomad
  $branchName = Get-BranchName

  Write-Output "`nUpdating masterlists' default branch to $branchName..."
  lomad -t $env:github_auth_token -a -d $branchName

  if ($lastexitcode -eq 0) {
    Write-Output "Masterlists' default branches have been updated."
  }
}
