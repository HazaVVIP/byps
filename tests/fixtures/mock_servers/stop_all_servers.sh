#!/bin/bash
# Script to stop all running mock servers

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${SCRIPT_DIR}/logs"
PID_FILE="${LOG_DIR}/pids.txt"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Stopping Mock Servers..."

if [ ! -f "$PID_FILE" ]; then
    echo -e "${RED}No PID file found at $PID_FILE${NC}"
    echo "Servers may not be running or were started manually."
    echo ""
    echo "Attempting to find and kill servers by port..."
    
    # Try to kill by port
    for port in 8000 8001 8002; do
        pid=$(lsof -ti:$port 2>/dev/null)
        if [ ! -z "$pid" ]; then
            echo "  Killing process on port $port (PID: $pid)"
            kill $pid 2>/dev/null
        fi
    done
    exit 0
fi

# Read PIDs and kill each process
while IFS= read -r pid; do
    if ps -p $pid > /dev/null 2>&1; then
        echo "  Stopping process $pid"
        kill $pid 2>/dev/null
        
        # Wait a moment and force kill if still running
        sleep 1
        if ps -p $pid > /dev/null 2>&1; then
            echo "  Force stopping process $pid"
            kill -9 $pid 2>/dev/null
        fi
    else
        echo "  Process $pid already stopped"
    fi
done < "$PID_FILE"

# Remove PID file
rm -f "$PID_FILE"

echo -e "${GREEN}All servers stopped!${NC}"
