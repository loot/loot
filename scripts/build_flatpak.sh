#!/bin/sh
# This script rebuilds LOOT, the docs and the MO files, creates an archive, then
# builds and installs the Flatpak package and creates a single-file bundle. It
# assumes that LOOT has already been built before and that all necessary
# dependencies have already been installed.
set -e

BRANCH_NAME=$(git rev-parse --abbrev-ref HEAD)
ARCHIVE_FILENAME=loot_$(git describe --tags --long --abbrev=7)_${BRANCH_NAME}.tar.xz
PACKAGE_FILENAME=loot_$(git describe --tags --long --abbrev=7)_${BRANCH_NAME}.flatpak

cd build
make LOOT

cd ..

export PATH="$PATH:$HOME/.local/bin"
sphinx-build -b html docs build/docs/html

python3 scripts/po_to_mo.py

echo "Creating archive..."
python3 scripts/archive.py . $BRANCH_NAME

cd build
echo "Creating flatpak package..."
rm -f loot.tar.xz
ln -s $ARCHIVE_FILENAME loot.tar.xz
flatpak-builder --repo=flatpak-repo --force-clean --disable-cache --install --user flatpak ../resources/linux/io.github.loot.loot.yml

echo "Creating flatpak bundle..."
flatpak build-bundle flatpak-repo $PACKAGE_FILENAME io.github.loot.loot
