# Mock Servers for Testing

This directory contains mock HTTP servers for testing various bypass techniques.

## Servers

### 1. Basic Server (`basic_server.py`)
**Port:** 8000

Tests basic path normalization and encoding bypasses:
- `/admin` → 403 Forbidden
- `/admin/` → 200 OK (trailing slash bypass)
- `/%61dmin` → 200 OK (URL encoding bypass)
- All other paths → 200 OK

**Usage:**
```bash
python3 basic_server.py
```

**Test with curl:**
```bash
curl http://127.0.0.1:8000/admin        # 403
curl http://127.0.0.1:8000/admin/       # 200
curl http://127.0.0.1:8000/%61dmin      # 200
```

### 2. WAF Server (`waf_server.py`)
**Port:** 8001

Simulates Web Application Firewall behavior with pattern blocking:
- Blocks: `../`, `<script>`, `union select`, `<iframe>`, `javascript:`, `onerror=`
- Returns Cloudflare-like headers and error pages
- Blocks both in URL and POST data

**Usage:**
```bash
python3 waf_server.py
```

**Test with curl:**
```bash
curl http://127.0.0.1:8001/normal                    # 200
curl http://127.0.0.1:8001/../etc/passwd             # 403 WAF block
curl 'http://127.0.0.1:8001/?q=<script>'             # 403 WAF block
curl 'http://127.0.0.1:8001/?q=union+select'         # 403 WAF block
```

### 3. Header Server (`header_server.py`)
**Port:** 8002

Tests header-based access control and bypasses:

#### `/admin` endpoint
- Requires `X-Forwarded-For: 127.0.0.1` or `X-Real-IP: 127.0.0.1`

#### `/restricted` endpoint
- Requires `X-Original-URL: /allowed` or `X-Rewrite-URL: /allowed`

#### `/combined` endpoint
- Requires both IP whitelist AND URL override headers

#### `/headers` endpoint
- Echoes all request headers for debugging

**Usage:**
```bash
python3 header_server.py
```

**Test with curl:**
```bash
# IP whitelist bypass
curl http://127.0.0.1:8002/admin                                    # 403
curl -H "X-Forwarded-For: 127.0.0.1" http://127.0.0.1:8002/admin   # 200

# URL override bypass
curl http://127.0.0.1:8002/restricted                               # 403
curl -H "X-Original-URL: /allowed" http://127.0.0.1:8002/restricted # 200

# Combined bypass
curl -H "X-Forwarded-For: 127.0.0.1" \
     -H "X-Original-URL: /allowed" \
     http://127.0.0.1:8002/combined                                 # 200

# View headers
curl http://127.0.0.1:8002/headers
```

## Running All Servers

### Start All Servers
```bash
./run_all_servers.sh
```

This will:
- Start all three servers in the background
- Create log files in `logs/` directory
- Save PIDs to `logs/pids.txt`

### Stop All Servers
```bash
./stop_all_servers.sh
```

Or manually:
```bash
kill $(cat logs/pids.txt)
```

## Requirements

- Python 3.8+
- No external dependencies (uses standard library only)

## Logs

When using `run_all_servers.sh`, logs are stored in:
- `logs/basic_server.log`
- `logs/waf_server.log`
- `logs/header_server.log`
- `logs/pids.txt` (process IDs)

## Testing Integration

These servers are designed to be used in automated tests:

```python
import subprocess
import time
import requests

# Start servers
subprocess.Popen(['python3', 'basic_server.py'])
time.sleep(1)

# Run tests
response = requests.get('http://127.0.0.1:8000/admin')
assert response.status_code == 403

response = requests.get('http://127.0.0.1:8000/admin/')
assert response.status_code == 200
```
