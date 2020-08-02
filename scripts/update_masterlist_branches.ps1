param (
  [Parameter(Mandatory=$True)]
  [string] $gitTagName,
  [Parameter(Mandatory=$True)]
  [string] $githubToken
)

function Get-BranchName($gitTagName) {
  $numbers = $gitTagName.Split('.')

  if ($numbers[0] -eq 0) {
    $numbers = $numbers[0..($numbers.length - 2)]
  } else {
    $numbers = @($numbers[0])
  }

  return "v" + ($numbers -join '.')
}

yarn global add lomad
$branchName = Get-BranchName $gitTagName

Write-Output "`nUpdating masterlists' default branch to $branchName..."
lomad -t $githubToken -a -d $branchName

if ($lastexitcode -eq 0) {
  Write-Output "Masterlists' default branches have been updated."
}
