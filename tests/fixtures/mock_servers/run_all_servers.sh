#!/bin/bash
# Script to run all mock servers for testing
# All servers run in background and log to separate files

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${SCRIPT_DIR}/logs"

# Create log directory
mkdir -p "$LOG_DIR"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Starting Mock Servers...${NC}"
echo "Log directory: $LOG_DIR"
echo ""

# Function to start a server
start_server() {
    local script_name=$1
    local port=$2
    local log_file="${LOG_DIR}/${script_name%.py}.log"
    
    # Check if port is already in use
    if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1; then
        echo -e "${YELLOW}Warning: Port $port already in use, skipping ${script_name}${NC}"
        return 1
    fi
    
    # Start the server in background
    python3 "${SCRIPT_DIR}/${script_name}" > "$log_file" 2>&1 &
    local pid=$!
    
    # Wait a moment for server to start
    sleep 1
    
    # Check if process is still running
    if ps -p $pid > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} ${script_name} started (PID: $pid, Port: $port, Log: $log_file)"
        echo $pid >> "${LOG_DIR}/pids.txt"
        return 0
    else
        echo -e "${RED}✗${NC} ${script_name} failed to start (check $log_file)"
        return 1
    fi
}

# Clear old PID file
rm -f "${LOG_DIR}/pids.txt"

# Start all servers
start_server "basic_server.py" 8000
start_server "waf_server.py" 8001
start_server "header_server.py" 8002

echo ""
echo -e "${GREEN}All servers started!${NC}"
echo ""
echo "Server URLs:"
echo "  - Basic Server:  http://127.0.0.1:8000"
echo "  - WAF Server:    http://127.0.0.1:8001"
echo "  - Header Server: http://127.0.0.1:8002"
echo ""
echo "To stop all servers, run:"
echo "  kill \$(cat ${LOG_DIR}/pids.txt)"
echo ""
echo "Or use the stop_all_servers.sh script (if available)"
