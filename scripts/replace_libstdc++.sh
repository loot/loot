#!/usr/bin/env sh
set -e

TARGET_DIR="$1"
DLL_NAME="libstdc++-6.dll"

if [ -z "$TARGET_DIR" ]
then
  echo "Please supply a target directory!"
  exit 1
fi

if [ -n "$WINEPATH" ]
then
  WINE_PATHS=$(echo "$WINEPATH" | tr ";" "\n")
  for WINE_PATH in $WINE_PATHS
  do
    if [ -e "$WINE_PATH/$DLL_NAME" ]
    then
      if [ -e "$TARGET_DIR/$DLL_NAME" ]
      then
        echo "Replacing the $DLL_NAME in $TARGET_DIR with the one in $WINE_PATH"
      fi

      cp -f "$WINE_PATH/$DLL_NAME" "$TARGET_DIR/$DLL_NAME"
      break
    fi
  done
fi
