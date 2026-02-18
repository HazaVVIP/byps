# Byps Developer Documentation

## ✅ Project Status: IMPLEMENTATION COMPLETE

The Byps project has been successfully implemented and is fully functional.

## What's Been Implemented

### 1. C++ Core Engine (✅ Complete)
- **Bypass Techniques**:
  - Path manipulation (trailing slash, encoding, case variation, path traversal)
  - Header forgery (X-Forwarded-For, X-Original-URL, Host manipulation)
  - URL encoding (single, double, triple, mixed, partial)
  - Unicode techniques (normalization, homographs, zero-width characters)
  - Protocol abuse (version manipulation, method variation)

- **Network Layer**:
  - HTTP/HTTPS client with TLS/SSL support
  - Connection pooling for efficiency
  - Request/response handling
  - OpenSSL integration

- **Common Utilities**:
  - Logger with multiple levels and file output
  - Error handling with custom exceptions
  - URL parsing and manipulation
  - String utilities (trim, split, case conversion)
  - Encoding/decoding (URL, base64, hex)

- **FFI Bridge**:
  - C API for Rust integration
  - Memory-safe interface
  - Version information

### 2. Rust CLI Layer (✅ Complete)
- **Command-line Interface**:
  - Scan command (full technique scan)
  - Test command (single technique)
  - Detect command (WAF detection - placeholder)
  - List command (available techniques)

- **Output Formatters**:
  - JSON (pretty-printed)
  - CSV (structured data)
  - HTML (beautiful reports with styling)
  - Terminal (colored, human-readable)

- **Configuration System**:
  - YAML config file support
  - Multiple strategies (default, aggressive, stealth)
  - Error handling and reporting

### 3. Resources (✅ Complete)
- **Wordlists**:
  - `paths.txt` - Common restricted paths
  - `headers.txt` - Header manipulation candidates
  - `methods.txt` - HTTP methods

- **Configuration Files**:
  - `default.yaml` - Balanced scanning
  - `aggressive.yaml` - Fast, high-concurrency
  - `stealth.yaml` - Slow, low-profile

### 4. Testing Infrastructure (✅ Complete)
- **Python Mock Servers**:
  - `basic_server.py` - Path bypass testing (port 8000)
  - `header_server.py` - Header bypass testing (port 8001)
  - `waf_server.py` - Mock WAF behavior (port 8002)

- **Build Scripts**:
  - `build.sh` - Complete build process
  - `setup.sh` - Environment setup
  - `test.sh` - Test runner

### 5. Documentation (✅ Complete)
- Comprehensive README with examples
- Implementation summary
- Demo walkthrough
- Build and usage instructions

## Quick Start for Developers

```bash
# Clone and build
git clone https://github.com/HazaVVIP/byps
cd byps
./scripts/build.sh

# Run tests
python3 tests/fixtures/mock_servers/basic_server.py &
./target/release/byps scan http://127.0.0.1:8000/admin -v
```

## Project Structure

```
byps/
├── cpp/                    # C++ core engine
│   ├── include/
│   │   ├── common/        # Logger, utils, error handling
│   │   ├── network/       # HTTP client, TLS, connection pool
│   │   └── techniques/    # Bypass implementations
│   ├── src/               # Implementation files
│   └── lib/               # FFI bridge
├── src/                   # Rust CLI layer
│   ├── main.rs           # Entry point
│   ├── cli.rs            # Argument parsing
│   ├── bridge.rs         # FFI bindings
│   └── output/           # Formatters
├── resources/            # Wordlists and configs
├── tests/                # Test infrastructure
└── scripts/              # Build scripts
```

## Building

```bash
# C++ library
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# Rust CLI
cargo build --release
```

## Testing

```bash
# Start mock server
python3 tests/fixtures/mock_servers/basic_server.py &

# Test commands
./target/release/byps list
./target/release/byps test http://127.0.0.1:8000/admin --technique path_bypass
./target/release/byps scan http://127.0.0.1:8000/admin -v --output terminal
```

## Contributing

When contributing:
1. Follow existing code style (C++17, Rust idioms)
2. Add tests for new features
3. Update documentation
4. Ensure all bypass techniques are for authorized security testing only

## Development Notes

- The project uses CMake for C++ and Cargo for Rust
- FFI bridge connects C++ core to Rust CLI
- All output formats are fully implemented
- Mock servers allow safe localhost testing
- Build system is fully integrated and working

---

## Archived Planning Documentation

Previous research and planning documents (roadmaps, architecture research, etc.) have been removed as the implementation is complete. For current documentation, refer to:

- [README.md](../README.md) - Main project documentation
- [IMPLEMENTATION_SUMMARY.md](../IMPLEMENTATION_SUMMARY.md) - Detailed implementation status
- [DEMO.md](../DEMO.md) - Usage examples

**Made with ❤️ for the security community**
