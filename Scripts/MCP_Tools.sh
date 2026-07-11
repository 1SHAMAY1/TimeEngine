#!/usr/bin/env bash
# MCP_Tools.sh — Cross-platform shell wrapper to call MCP tools via HTTP POST requests
# Usage: ./Scripts/MCP_Tools.sh <command_name> [args...]

PORT=3000
URL="http://127.0.0.1:$PORT/message"

CMD=$1
shift

if [ -z "$CMD" ]; then
    echo "Usage: $0 <command> [args...]"
    echo ""
    echo "--- Existing Tools ---"
    echo "  info                       - Get engine info"
    echo "  get_scene                  - Get scene hierarchy"
    echo "  create_entity <name>       - Create a named entity"
    echo "  destroy_entity <id>        - Destroy an entity by ID"
    echo "  create_sprite <name> <path>- Create a sprite asset"
    echo "  mkdir <path>               - Create a folder"
    echo "  rm <path>                  - Delete file or folder"
    echo ""
    echo "--- New Tools ---"
    echo "  get_modes                  - Get list of editor modes"
    echo "  set_mode <mode_name>       - Switch current editor mode"
    echo "  screenshot                 - Capture viewport (temp file)"
    echo "  send_key <keycode>         - Simulate keyboard key press"
    echo "  send_click <mousecode>     - Simulate mouse click"
    echo "  select <id>                - Select entity by ID"
    echo "  set_properties <id> <json> - Set properties on entity components"
    exit 1
fi

case "$CMD" in
    # Existing Tools
    info)
        PAYLOAD='{"jsonrpc":"2.0","method":"tools/call","params":{"name":"get_engine_info","arguments":{}},"id":1}'
        ;;
    get_scene)
        PAYLOAD='{"jsonrpc":"2.0","method":"tools/call","params":{"name":"get_scene_hierarchy","arguments":{}},"id":1}'
        ;;
    create_entity)
        if [ -z "$1" ]; then echo "Usage: create_entity <name>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"create_entity\",\"arguments\":{\"name\":\"$1\"}},\"id\":1}"
        ;;
    destroy_entity)
        if [ -z "$1" ]; then echo "Usage: destroy_entity <id>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"destroy_entity\",\"arguments\":{\"id\":$1}},\"id\":1}"
        ;;
    create_sprite)
        if [ -z "$1" ] || [ -z "$2" ]; then echo "Usage: create_sprite <name> <path>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"create_sprite\",\"arguments\":{\"name\":\"$1\",\"path\":\"$2\"}},\"id\":1}"
        ;;
    mkdir)
        if [ -z "$1" ]; then echo "Usage: mkdir <path>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"create_directory\",\"arguments\":{\"path\":\"$1\"}},\"id\":1}"
        ;;
    rm)
        if [ -z "$1" ]; then echo "Usage: rm <path>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"delete_file_or_directory\",\"arguments\":{\"path\":\"$1\"}},\"id\":1}"
        ;;

    # New Tools
    get_modes)
        PAYLOAD='{"jsonrpc":"2.0","method":"tools/call","params":{"name":"get_editor_modes","arguments":{}},"id":1}'
        ;;
    set_mode)
        if [ -z "$1" ]; then echo "Usage: set_mode <mode_name>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"set_editor_mode\",\"arguments\":{\"mode\":\"$1\"}},\"id\":1}"
        ;;
    screenshot)
        PAYLOAD='{"jsonrpc":"2.0","method":"tools/call","params":{"name":"get_viewport_screenshot","arguments":{}},"id":1}'
        ;;
    delete_screenshot)
        PAYLOAD='{"jsonrpc":"2.0","method":"tools/call","params":{"name":"delete_screenshot","arguments":{}},"id":1}'
        ;;
    send_key)
        if [ -z "$1" ]; then echo "Usage: send_key <keycode>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"send_editor_input\",\"arguments\":{\"type\":\"key_press\",\"code\":$1}},\"id\":1}"
        ;;
    send_click)
        if [ -z "$1" ]; then echo "Usage: send_click <mousecode>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"send_editor_input\",\"arguments\":{\"type\":\"mouse_press\",\"code\":$1}},\"id\":1}"
        ;;
    select)
        if [ -z "$1" ]; then echo "Usage: select <id>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"select_entity\",\"arguments\":{\"id\":$1}},\"id\":1}"
        ;;
    set_properties)
        if [ -z "$1" ] || [ -z "$2" ]; then echo "Usage: set_properties <id> <json_properties>"; exit 1; fi
        SHIFTED_ARGS="${@:2}"
        # Build payload using python JSON builder script
        PAYLOAD=$(python Scripts/parse_json.py "$1" "$SHIFTED_ARGS")
        ;;
    add_component)
        if [ -z "$1" ] || [ -z "$2" ]; then echo "Usage: add_component <id> <component_type>"; exit 1; fi
        PAYLOAD="{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"params\":{\"name\":\"add_component\",\"arguments\":{\"id\":$1,\"type\":\"$2\"}},\"id\":1}"
        ;;
    *)
        echo "Unknown command: $CMD"
        exit 1
        ;;
esac

curl.exe -s -X POST "$URL" -H "Content-Type: application/json" -d "$PAYLOAD"
echo ""
