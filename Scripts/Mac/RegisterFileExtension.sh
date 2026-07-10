#!/bin/bash
# RegisterFileExtension.sh (Mac)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

# Search for TimeEditor executable
ENGINE_EXE=""
for path in \
    "Bin/Debug-macosx-x86_64/TimeEditor/TimeEditor" \
    "Bin/Release-macosx-x86_64/TimeEditor/TimeEditor" \
    "Bin/Debug-macosx-arm64/TimeEditor/TimeEditor" \
    "Bin/Release-macosx-arm64/TimeEditor/TimeEditor"; do
    if [ -f "$ROOT_DIR/$path" ]; then
        ENGINE_EXE="$ROOT_DIR/$path"
        break
    fi
done

if [ -z "$ENGINE_EXE" ]; then
    ENGINE_EXE=$(find "$ROOT_DIR/Bin" -type f -name "TimeEditor" | grep -v "Intermediate" | head -n 1)
fi

if [ -n "$ENGINE_EXE" ] && [ -f "$ENGINE_EXE" ]; then
    echo "[TimeEngine] Registering .teproj file extension..."
    echo "[TimeEngine] Found executable: $ENGINE_EXE"
    chmod +x "$ENGINE_EXE"
    "$ENGINE_EXE" --register
    echo "[TimeEngine] Registration process completed."
else
    echo "[TimeEngine] ERROR: TimeEditor executable not found!"
    echo "[TimeEngine] Please build the TimeEditor project first before running this script."
    read -p "Press Enter to continue..."
fi
