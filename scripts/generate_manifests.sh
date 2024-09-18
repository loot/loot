#!/usr/bin/sh
# Generate Flatpak manifest files for cargo and pip dependencies needed to build LOOT from source.
set -e

APP_MANIFEST="../../resources/linux/io.github.loot.loot.yml"

generate_cargo_manifest() {
    REPO="$1"

    URL="$(grep -Po "https://.+/$REPO/.+" "$APP_MANIFEST")"
    VERSION="${URL##*/}"
    VERSION="${VERSION%.tar.gz}"

    echo "Downloading and extracting $URL..."
    curl -sSfL "$URL" | tar xz

    echo "Generating manifest for $REPO $VERSION..."
    ./flatpak-cargo-generator.py "$REPO-$VERSION/Cargo.lock" -o "$REPO.json"
}

mkdir -p build/flatpak-manifests
cd build/flatpak-manifests

curl -sSfLO https://raw.githubusercontent.com/flatpak/flatpak-builder-tools/8bdf4f4892fefedd316035dbc041a65ba6a4dec8/cargo/flatpak-cargo-generator.py
curl -sSfLO https://raw.githubusercontent.com/flatpak/flatpak-builder-tools/8bdf4f4892fefedd316035dbc041a65ba6a4dec8/pip/flatpak-pip-generator

chmod u+x flatpak-cargo-generator.py
chmod u+x flatpak-pip-generator

echo "Creating and activating venv..."
python3 -m venv .venv
. .venv/bin/activate

echo "Installing python dependencies..."
python3 -m pip install aiohttp requirements-parser setuptools toml pyyaml

echo "Installing Flatpak dependencies..."
# Read the runtime from the app manifest to avoid repeating it here or
# unnecessarily downloading a different runtime.
RUNTIME="$(python3 -c "import yaml; content = yaml.safe_load(open('$APP_MANIFEST', 'r')); print('{}//{}'.format(content['sdk'], content['runtime-version']))")"

flatpak --user remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak --user install -y flathub "$RUNTIME"

echo "Generating manifest for docs..."
./flatpak-pip-generator --runtime="$RUNTIME" --requirements-file ../../docs/requirements.txt --output docs --cleanup=all --build-isolation

generate_cargo_manifest cbindgen
generate_cargo_manifest esplugin
generate_cargo_manifest libloadorder
generate_cargo_manifest loot-condition-interpreter
