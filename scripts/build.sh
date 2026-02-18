#!/bin/bash
# Build script for Byps project

set -e  # Exit on error

echo "=== Building Byps Project ==="
echo ""

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Build C++ core
echo -e "${BLUE}Building C++ core...${NC}"
mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

echo -e "${GREEN}✓ C++ core built successfully${NC}"
echo ""

cd ..

# Build Rust CLI
echo -e "${BLUE}Building Rust CLI...${NC}"
cargo build --release

echo -e "${GREEN}✓ Rust CLI built successfully${NC}"
echo ""

# Summary
echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Binary location: target/release/byps"
echo ""
echo "Run with: ./target/release/byps --help"
echo ""
