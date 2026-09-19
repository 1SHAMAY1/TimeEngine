#!/usr/bin/env bash
# ==============================================================================
# TimeEngine Headless Test Runner (macOS)
# Runs tests against the current built binary without compiling.
# ==============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/../.."

EXE_DEBUG="$ROOT_DIR/Artifacts/Bin/Debug-macosx-x86_64/TimeEditor/TimeEditor"
EXE_RELEASE="$ROOT_DIR/Artifacts/Bin/Release-macosx-x86_64/TimeEditor/TimeEditor"

TARGET_EXE=""
if [ -f "$EXE_DEBUG" ]; then
    TARGET_EXE="$EXE_DEBUG"
elif [ -f "$EXE_RELEASE" ]; then
    TARGET_EXE="$EXE_RELEASE"
fi

if [ -z "$TARGET_EXE" ]; then
    echo "[ERROR] TimeEngine executable not found in Artifacts/Bin/Debug-macosx-x86_64 or Artifacts/Bin/Release-macosx-x86_64."
    echo "Please build the project first using Scripts/Mac/BuildDebug.sh"
    exit 1
fi

echo "[TimeEngine] Executing headless tests with: $TARGET_EXE"
"$TARGET_EXE" --headless --test "$@"
exit $?
