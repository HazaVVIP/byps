#!/bin/bash
# Setup development environment

set -e

echo "=== Setting up Byps Development Environment ==="
echo ""

# Check for required tools
echo "Checking for required tools..."

command -v cmake >/dev/null 2>&1 || { echo "cmake is required but not installed. Aborting." >&2; exit 1; }
command -v cargo >/dev/null 2>&1 || { echo "cargo (Rust) is required but not installed. Aborting." >&2; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo "python3 is required but not installed. Aborting." >&2; exit 1; }

echo "✓ All required tools found"
echo ""

# Make scripts executable
echo "Making scripts executable..."
chmod +x scripts/*.sh
chmod +x tests/fixtures/mock_servers/*.py

echo "✓ Scripts are now executable"
echo ""

echo "=== Setup Complete ===="
echo ""
echo "Next steps:"
echo "1. Build the project: ./scripts/build.sh"
echo "2. Run tests: ./scripts/test.sh"
echo "3. Start a test server: python3 tests/fixtures/mock_servers/basic_server.py"
echo "4. Try the tool: ./target/release/byps scan http://127.0.0.1:8000/admin"
echo ""
