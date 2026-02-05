#!/usr/bin/env sh
set -e

# Install wine-mono while suppressing the dialog about wine-mono not being
# installed, to prevent the dialog being displayed while running the Inno
# setup installer or compiler.
WINEDLLOVERRIDES="mscoree=;mshtml=;" wine msiexec -i /home/vscode/wine-mono-7.4.0-x86.msi /quiet /qn

# Install Inno Setup to "C:/Program Files (x86)/Inno Setup 6" in the Wine prefix.
# This fails if there is no display, even though it doesn't display anything.
wine /home/vscode/innosetup-6.7.0.exe /VERYSILENT /ALLUSERS /SUPPRESSMSGBOXES
