#!/usr/bin/env sh
set -e

GIT_CONFIG_FILE=".git/config.devcontainer.nonlocal"

copy_non_empty_entries() {
  SCOPE="$1"

  git config list "--$SCOPE" | while read -r ENTRY
  do
    NAME="${ENTRY%%=*}"
    VALUE="${ENTRY#*=}"
    if [ -n "$VALUE" ]
    then
      git config set --file "$GIT_CONFIG_FILE" "$NAME" "$VALUE"
    fi
  done
}

rm "$GIT_CONFIG_FILE"

# Copy all non-local Git config into a file that will be available in the
# container, so that it can then be selectively applied as global config within
# the dev container.
copy_non_empty_entries system
copy_non_empty_entries global
