#!/usr/bin/sh
# Generate Flatpak manifest files for cargo and pip dependencies needed to build LOOT from source.
set -e

APP_MANIFEST="../../resources/linux/io.github.loot.loot.yml"
BUILD_DIR="build/flatpak-manifests"

READ_RUNTIME_PY="import yaml; content = yaml.safe_load(open('$APP_MANIFEST', 'r')); print('{}//{}'.format(content['sdk'], content['runtime-version']))"
YAML_TO_JSON_PY="import yaml; import json; print(json.dumps(yaml.safe_load(open('$APP_MANIFEST', 'r'))))"
PYYAML_SPECIFIER="pyyaml==6.0.3"

generate_libloot_cargo_manifest() {
    echo "Generating manifest for libloot..."
    APP_MANIFEST_JSON="$(echo "$YAML_TO_JSON_PY" | uv run --with "$PYYAML_SPECIFIER" -- -)"

    COMMIT_HASH="$(echo "$APP_MANIFEST_JSON" | jq -r '.modules[] | select(.name == "libloot") | .sources[] | objects | select(.type == "git") | .commit')"

    CARGO_LOCK_URL="https://raw.githubusercontent.com/loot/libloot/${COMMIT_HASH}/Cargo.lock"

    echo "Downloading and extracting $CARGO_LOCK_URL..."
    curl -sSfLO "$CARGO_LOCK_URL"

    echo "Generating manifest for libloot commit $COMMIT_HASH..."
    uv run --locked -- ./flatpak-cargo-generator.py "Cargo.lock" -o "libloot.json"
}

mkdir -p build/flatpak-manifests
cd build/flatpak-manifests

curl -sSfLO https://raw.githubusercontent.com/flatpak/flatpak-builder-tools/f03a673abe6ce189cea1c2857e2b44af2dd79d1f/cargo/flatpak-cargo-generator.py
echo "b373c8ab1a05378ec5d8ed0645c7b127bcec7d2f7a1798694fbc627d570d856c flatpak-cargo-generator.py" | sha256sum -c

curl -sSfLO https://raw.githubusercontent.com/flatpak/flatpak-builder-tools/ea92dc22ab7e4ab44133407b883c9a6792e54302/pip/flatpak-pip-generator.py
echo "3b3c0c06940cd3ab77c3ad2990f38ad8e421b4c564561ad275b64c4f020f6397 flatpak-pip-generator.py" | sha256sum -c

cp -f ../../resources/linux/flatpak-cargo-generator.py.lock ./
cp -f ../../resources/linux/flatpak-pip-generator.py.lock ./

echo "Installing Flatpak dependencies..."
# Read the runtime from the app manifest to avoid repeating it here or
# unnecessarily downloading a different runtime.
RUNTIME="$(echo "$READ_RUNTIME_PY" | uv run --with "$PYYAML_SPECIFIER" -- -)"

flatpak --user remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak --user install -y flathub "$RUNTIME"

echo "Generating manifest for docs..."
uv run --locked -- ./flatpak-pip-generator.py --runtime="$RUNTIME" --pyproject-file ../../docs/pyproject.toml --output docs --cleanup=all

generate_libloot_cargo_manifest
