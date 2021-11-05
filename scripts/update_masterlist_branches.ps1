param (
  [Parameter(Mandatory=$True)]
  [string] $branchName,
  [Parameter(Mandatory=$True)]
  [string] $githubToken
)

npm install --global lomad

Write-Output "`nUpdating masterlists' default branch to $branchName..."
lomad -t $githubToken -a -d $branchName

if ($lastexitcode -eq 0) {
  Write-Output "Masterlists' default branches have been updated."
}
