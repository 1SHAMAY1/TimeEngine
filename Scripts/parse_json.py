import sys
import json
import re

def repair_json(raw):
    # 1. Wrap unquoted keys in quotes
    # e.g., TransformComponent: -> "TransformComponent":
    s = re.sub(r'([a-zA-Z_][a-zA-Z0-9_]*)\s*:', r'"\1":', raw)
    
    # 2. Wrap unquoted values (which don't start with { or [) in quotes
    # e.g., : 0.0 -2.5 0.0 -> : "0.0 -2.5 0.0"
    # E.g., : 3.5 -> : "3.5"
    s = re.sub(r':\s*([^\{\}\[\s,][^\{\}\[\],]*[^\{\}\[\s,]|[^\{\}\[\s,])', lambda m: ': "' + m.group(1).strip() + '"', s)
    
    return s

if __name__ == '__main__':
    id_val = int(sys.argv[1])
    # Combine all remaining arguments into the properties string
    props_raw = ' '.join(sys.argv[2:])
    
    # Remove any backslashes introduced by double escapes
    props_raw = props_raw.replace('\\"', '"').replace('\\\\"', '"')
    
    # Try parsing directly first
    try:
        parsed = json.loads(props_raw)
    except Exception:
        # If parsing fails, try to repair it
        repaired = repair_json(props_raw)
        try:
            parsed = json.loads(repaired)
        except Exception as e:
            # Fallback if both fail
            parsed = props_raw

    payload = {
        'jsonrpc': '2.0',
        'method': 'tools/call',
        'params': {
            'name': 'set_entity_properties',
            'arguments': {
                'id': id_val,
                'properties': json.dumps(parsed) if isinstance(parsed, dict) else parsed
            }
        },
        'id': 1
    }
    print(json.dumps(payload))
