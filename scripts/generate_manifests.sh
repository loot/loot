#!/usr/bin/sh
# Generate Flatpak manifest files for cargo and pip dependencies needed to build LOOT from source.
set -e

APP_MANIFEST="../../resources/linux/io.github.loot.loot.yml"
BUILD_DIR="build/flatpak-manifests"

generate_libloot_cargo_manifest() {
    echo "Generating manifest for libloot..."
    APP_MANIFEST_JSON="$(python3 -c "import yaml; import json; print(json.dumps(yaml.safe_load(open('$APP_MANIFEST', 'r'))))")"

    echo "PRINTING APP_MANIFEST_JSON:"
    echo "$APP_MANIFEST_JSON"
    COMMIT_HASH="$(echo "$APP_MANIFEST_JSON" | jq -r '.modules[] | select(.name == "libloot") | .sources[] | objects | select(.type == "git") | .commit')"

    CARGO_LOCK_URL="https://raw.githubusercontent.com/loot/libloot/${COMMIT_HASH}/Cargo.lock"

    echo "Downloading and extracting $CARGO_LOCK_URL..."
    curl -sSfLO "$CARGO_LOCK_URL"

    echo "Generating manifest for libloot commit $COMMIT_HASH..."
    ./flatpak-cargo-generator.py "Cargo.lock" -o "libloot.json"
}

mkdir -p build/flatpak-manifests
cd build/flatpak-manifests

curl -sSfLO https://raw.githubusercontent.com/flatpak/flatpak-builder-tools/8bdf4f4892fefedd316035dbc041a65ba6a4dec8/cargo/flatpak-cargo-generator.py
echo "be155d00839c781c5d3176891facab2e5b3a423e88c73c7799a6b7e5d3a3786d flatpak-cargo-generator.py" | sha256sum -c

curl -sSfLO https://raw.githubusercontent.com/flatpak/flatpak-builder-tools/ea92dc22ab7e4ab44133407b883c9a6792e54302/pip/flatpak-pip-generator.py
echo "3b3c0c06940cd3ab77c3ad2990f38ad8e421b4c564561ad275b64c4f020f6397 flatpak-pip-generator.py" | sha256sum -c

chmod u+x flatpak-cargo-generator.py
chmod u+x flatpak-pip-generator.py

echo "Creating and activating venv..."
python3 -m venv .venv
. .venv/bin/activate

echo "Installing python dependencies..."
python3 -m pip install aiohttp requirements-parser setuptools toml tomli pyyaml

echo "Installing Flatpak dependencies..."
# Read the runtime from the app manifest to avoid repeating it here or
# unnecessarily downloading a different runtime.
RUNTIME="$(python3 -c "import yaml; content = yaml.safe_load(open('$APP_MANIFEST', 'r')); print('{}//{}'.format(content['sdk'], content['runtime-version']))")"

flatpak --user remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak --user install -y flathub "$RUNTIME"

echo "Generating manifest for docs..."
./flatpak-pip-generator.py --runtime="$RUNTIME" --pyproject-file ../../docs/pyproject.toml --output docs --cleanup=all

generate_libloot_cargo_manifest
