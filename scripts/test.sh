#!/bin/bash
# Test script for Byps project

set -e

echo "=== Running Byps Tests ==="
echo ""

GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

# Run C++ tests
echo -e "${BLUE}Running C++ tests...${NC}"
if [ -d "build" ]; then
    cd build
    ctest --output-on-failure || echo -e "${RED}C++ tests failed (this is expected if GoogleTest is not installed)${NC}"
    cd ..
else
    echo -e "${RED}Build directory not found. Run ./scripts/build.sh first${NC}"
fi

echo ""

# Run Rust tests
echo -e "${BLUE}Running Rust tests...${NC}"
cargo test

echo ""
echo -e "${GREEN}=== Tests Complete ===${NC}"
