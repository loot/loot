$docsRepoUrl = 'https://github.com/loot/loot.github.io.git'
$docsRepoPath = 'C:\projects\loot.github.io'
$docsIndex = 'docs/index.html'
$sourceFolder = $env:APPVEYOR_BUILD_FOLDER + '/docs'
$destinationFolder = 'docs/' + $env:APPVEYOR_REPO_TAG_NAME

$searchOption = '<option value="([^"]+)" selected>([^<]+)</option>'
$replaceOption = '<option value="$1">$2</option>'

$searchSelect = '(<select id="versionSelect">)'
$replaceSelect = '$1' + "`n" + '    <option value="' + $env:APPVEYOR_REPO_TAG_NAME + '" selected>v' + $env:APPVEYOR_REPO_TAG_NAME + '</option>'

$docsFiles = @(
  'images'
  'licenses'
  'LOOT Readme.html'
)

$commitMessage = 'Add v' + $env:APPVEYOR_REPO_TAG_NAME + " release docs`n`n From " + $env:APPVEYOR_REPO_NAME + '#' + $env:APPVEYOR_REPO_COMMIT

if ($env:APPVEYOR_REPO_TAG -eq 'true') {
  cd 'C:\projects'
  git clone $docsRepoUrl

  # Edit the docs index to list and select by default the new release docs.  | %{$_ -replace $searchSelect, $replaceSelect}
  cd $docsRepoPath
  $newDocsIndex = (cat $docsIndex) | %{$_ -replace $searchOption, $replaceOption} | %{$_ -replace $searchSelect, $replaceSelect} | %{$_ -replace "`n$", ''}
  [IO.File]::WriteAllLines($docsRepoPath + '\' + $docsIndex, $newDocsIndex)

  # Add the docs to their subfolder.
  foreach($file in $docsFiles) {
    cp $($sourceFolder + '/' + $file) $($destinationFolder + '/' + $file) -Force -recurse
  }

  # Configure Git credentials
  git config credential.helper store
  Add-Content "$env:USERPROFILE\.git-credentials" "https://$($env:access_token):x-oauth-basic@github.com`n"
  git config user.email "oliver.hamlet+wrinklyrobot@gmail.com"
  git config user.name "WrinklyRobot"

  # Commit and push the changes.
  git add $docsIndex $destinationFolder
  git commit -m $commitMessage
  git push
}
