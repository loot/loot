#!/usr/bin/sh
# This script builds and installs the Flatpak package and creates a single-file
# bundle. It assumes that the generate_manifests.sh script has already been run.
set -e

PACKAGE_FILENAME="$1"

if [ -z "$PACKAGE_FILENAME" ]
then
    BRANCH_NAME="$(git rev-parse --abbrev-ref HEAD)"
    DESCRIPTION="$(git describe --tags --long --abbrev=7)"
    PACKAGE_FILENAME="loot_${DESCRIPTION}_${BRANCH_NAME}.flatpak"
    PACKAGE_FILENAME="$(echo $PACKAGE_FILENAME | sed 's/[\/<>\"|]/_/g')"
fi

cd build

echo "Creating flatpak package..."
flatpak-builder --install-deps-from flathub --user --ccache --force-clean --install --repo=flatpak-repo flatpak ../resources/linux/io.github.loot.loot.yml

echo "Creating flatpak bundle..."
flatpak build-bundle flatpak-repo "$PACKAGE_FILENAME" io.github.loot.loot --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
