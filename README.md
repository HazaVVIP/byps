# Byps - WAF Bypass Testing Tool

A high-performance CLI tool written in C++ with a Rust frontend for testing and bypassing Web Application Firewalls (WAFs) and access control restrictions (401/403 errors).

## ⚠️ Legal Disclaimer

**FOR AUTHORIZED TESTING ONLY**

This tool is designed for security professionals to test systems they have explicit permission to test. Unauthorized use is illegal.

- ✅ Use only on systems you own or have written permission to test
- ✅ Use for security research and penetration testing with authorization  
- ❌ Never use against production systems without permission
- ❌ Never use for malicious purposes

## ✨ Features

### Bypass Techniques
- **Path Manipulation**: Trailing slash, URL encoding, case variation, path traversal
- **Header Forgery**: X-Forwarded-For, X-Original-URL, Host header manipulation
- **URL Encoding**: Single, double, triple encoding, mixed encoding
- **Unicode Techniques**: Normalization variations, homograph substitution, zero-width characters
- **Protocol Abuse**: HTTP version manipulation, method variation

### Performance & Architecture
- **High Performance**: C++ core engine for maximum speed
- **Network Layer**: Built-in HTTP/HTTPS client with TLS/SSL support
- **Connection Pooling**: Efficient connection reuse for faster scanning
- **Safe CLI**: Rust-based CLI interface with memory safety
- **Multiple Output Formats**: JSON, CSV, HTML, and colored terminal output

### Configuration
- **Pre-configured Strategies**: Default, Aggressive, and Stealth modes
- **Wordlists**: Built-in paths, headers, and HTTP methods
- **Flexible**: Easy customization via YAML config files

## 🚀 Quick Start

### Prerequisites

- CMake 3.15+
- C++17 compiler (g++, clang)
- Rust 1.70+
- OpenSSL development libraries

```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential libssl-dev

# macOS
brew install cmake openssl
```

### Build

```bash
# 1. Build C++ core library
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# 2. Build Rust CLI
cargo build --release
```

### Usage

```bash
# List available bypass techniques
./target/release/byps list

# Test a specific technique
./target/release/byps test http://example.com/admin --technique path_bypass

# Full scan with all techniques
./target/release/byps scan http://example.com/admin

# Save results to file
./target/release/byps scan http://example.com/admin -o results.html --output html

# Use stealth mode
./target/release/byps scan http://example.com/admin --strategy stealth

# Verbose output
./target/release/byps scan http://example.com/admin -v --output terminal
```

## 📖 Examples

### Basic Scan
```bash
$ ./target/release/byps scan http://127.0.0.1:8000/admin -v --output terminal

Byps - WAF Bypass Testing Tool
Version: 0.1.0

Scanning: http://127.0.0.1:8000/admin
=== Scan Results ===
Target URL: "http://127.0.0.1:8000/admin"

Variations Found: 10
  1. /admin/
  2. /admin//
  3. /admin/.
  4. /admin/./
  5. %2Fadmin
  6. %252Fadmin
  7. %25252Fadmin
  8. %2fadmin
  9. /%61%64%6d%69%6e
  10. /ADMIN
```

### Test Specific Technique
```bash
$ ./target/release/byps test http://example.com/secret --technique url_encoding -o results.json

# Output: JSON with all encoding variations
```

### HTML Report
```bash
$ ./target/release/byps scan http://example.com/api --output html -o report.html

# Generates a beautiful HTML report with styling
```

## 🧪 Testing

The project includes Python mock servers for safe localhost testing:

```bash
# Start basic path bypass server (port 8000)
python3 tests/fixtures/mock_servers/basic_server.py &

# Start header bypass server (port 8001)
python3 tests/fixtures/mock_servers/header_server.py &

# Start mock WAF server (port 8002)
python3 tests/fixtures/mock_servers/waf_server.py &

# Test against mock servers
./target/release/byps scan http://127.0.0.1:8000/admin
```

## 📁 Project Structure

```
byps/
├── cpp/                    # C++ core engine
│   ├── include/           # Header files
│   │   ├── common/        # Utilities, logger, error handling
│   │   ├── network/       # HTTP client, TLS, connection pool
│   │   └── techniques/    # Bypass technique implementations
│   ├── src/               # Implementation files
│   └── lib/               # FFI bridge (byps_core.cpp)
├── src/                   # Rust CLI layer
│   ├── main.rs           # CLI entry point
│   ├── cli.rs            # Command-line argument parsing
│   ├── bridge.rs         # FFI bindings to C++ core
│   └── output/           # Output formatters (JSON, CSV, HTML, Terminal)
├── resources/            # Wordlists and configurations
│   ├── wordlists/        # paths.txt, headers.txt, methods.txt
│   └── configs/          # default.yaml, aggressive.yaml, stealth.yaml
├── tests/                # Test infrastructure
│   ├── cpp/              # C++ unit tests
│   └── fixtures/         # Mock servers for testing
└── CMakeLists.txt        # C++ build configuration
```

## 🔧 Configuration

Configuration files are located in `resources/configs/`:

- `default.yaml` - Balanced scanning with moderate speed
- `aggressive.yaml` - Fast, high-concurrency scanning
- `stealth.yaml` - Slow, low-profile scanning to avoid detection

Example config:
```yaml
scan:
  max_threads: 10
  timeout_ms: 30000
  delay_ms: 100
  
techniques:
  path_bypass: true
  header_forge: true
  url_encoding: true
  unicode: true
  protocol_abuse: true
```

## 🛠️ Development

### Implementation Status

- ✅ Project structure and build system
- ✅ Common utilities (logger, error handling, utils)
- ✅ Path bypass techniques
- ✅ Header forgery techniques
- ✅ Encoding techniques (URL, HTML, Unicode)
- ✅ Unicode normalization and homographs
- ✅ Protocol abuse basics
- ✅ Network layer (HTTP/HTTPS client with TLS)
- ✅ Connection pooling
- ✅ C API / FFI bridge
- ✅ Rust CLI interface
- ✅ All output formatters (JSON, CSV, HTML, Terminal)
- ✅ Python mock test servers
- ✅ Resources (wordlists, configs)
- ✅ End-to-end testing

### Future Enhancements
- [ ] WAF detection engine
- [ ] Request smuggling techniques
- [ ] Advanced timing attack techniques
- [ ] Complete integration tests
- [ ] Performance benchmarks

## 📚 Documentation

- Main documentation is in this README
- Mock server usage: `tests/fixtures/mock_servers/README.md`
- Implementation details: `IMPLEMENTATION_SUMMARY.md`
- Demo walkthrough: `DEMO.md`

## 🤝 Contributing

Contributions are welcome! Please ensure:
1. Code follows existing style
2. Tests pass (`cargo test`, C++ tests)
3. Documentation is updated
4. Changes are for authorized security testing purposes

## 📄 License

MIT License - see LICENSE file

---

**Made with ❤️ for the security community**

**Status**: ✅ Fully functional - Ready for authorized testing
