#!/bin/bash
# GenerateProjectFiles.sh (Linux Default -> GCC)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
exec "$SCRIPT_DIR/GCC/GenerateProjectFiles.sh" "$@"