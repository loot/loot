#!/usr/bin/env sh
set -e

# Install wine-mono while suppressing the dialog about wine-mono not being
# installed, to prevent the dialog being displayed while running the Inno
# setup installer or compiler.
WINEDLLOVERRIDES="mscoree=;mshtml=;" wine msiexec -i /home/vscode/wine-mono-7.4.0-x86.msi /quiet /qn

# Install Inno Setup to "C:/Program Files (x86)/Inno Setup 6" in the Wine prefix.
# This fails if there is no display, even though it doesn't display anything.
wine /home/vscode/innosetup-6.7.0.exe /VERYSILENT /ALLUSERS /SUPPRESSMSGBOXES

# This is the same value as in initialize.sh
GIT_CONFIG_FILE=".git/config.devcontainer.nonlocal"

copy_from_non_local() {
  KEY="$1"
  if git config get --file "$GIT_CONFIG_FILE" "$KEY" && ! git config get --global "$KEY"
  then
    git config set --global "$KEY" "$(git config get --file "$GIT_CONFIG_FILE" "$KEY")"
  fi
}

# Copy the config from GIT_CONFIG_FILE into global config if not already set.
copy_from_non_local gpg.format
copy_from_non_local gpg.ssh.defaultKeyCommand
copy_from_non_local user.signingkey
copy_from_non_local commit.gpgsign
