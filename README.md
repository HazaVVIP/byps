# Byps - WAF Bypass Testing Tool

A high-performance CLI tool written in C++ with a Rust frontend for testing and bypassing Web Application Firewalls (WAFs) and access control restrictions (401/403 errors).

## ⚠️ Legal Disclaimer

**FOR AUTHORIZED TESTING ONLY**

This tool is designed for security professionals to test systems they have explicit permission to test. Unauthorized use is illegal.

- ✅ Use only on systems you own or have written permission to test
- ✅ Use for security research and penetration testing with authorization  
- ❌ Never use against production systems without permission
- ❌ Never use for malicious purposes

## Development Status

✅ **Core implementation complete** - Basic functionality working  
🚧 **Advanced features in progress** - WAF detection, request smuggling, etc.

### 📚 Research Documentation

All comprehensive research and planning documentation is available in the **[Developer](./Developer/)** directory:

- **[Quick Start Guide](./Developer/QUICK_START.md)** - Start here for navigation
- **[Research Plan](./Developer/RESEARCH_PLAN.md)** - What we're researching
- **[Development Roadmap](./Developer/DEVELOPMENT_ROADMAP.md)** - 21-week implementation plan
- **[Testing Strategy](./Developer/TESTING_STRATEGY.md)** - How we'll test (Python localhost servers)
- **[Architecture Research](./Developer/ARCHITECTURE_RESEARCH.md)** - System design and patterns
- **[Technique Research](./Developer/TECHNIQUE_RESEARCH.md)** - 20+ bypass techniques
- **[Technology Stack](./Developer/TECHNOLOGY_STACK.md)** - C++17, Rust, libraries

### 📋 Project Structure

See [DIR.txt](./DIR.txt) for the complete planned directory structure.

## Features

- **Multiple Bypass Techniques**:
  - Path manipulation (trailing slash, URL encoding, case variation)
  - Header forgery (X-Forwarded-For, X-Original-URL, Host header)
  - Protocol abuse (HTTP version, method variation)
  - Encoding techniques (URL encoding, HTML entities)

- **High Performance**: C++ core for maximum speed
- **Safe CLI**: Rust-based CLI interface
- **Flexible Output**: JSON and colored terminal output
- **Testing Infrastructure**: Python mock servers for safe testing

## Quick Start

### Build

```bash
# Build C++ core
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# Build Rust CLI
cargo build --release
```

### Test

```bash
# Start a test server
python3 tests/fixtures/mock_servers/basic_server.py &

# Run a scan
./target/release/byps scan http://127.0.0.1:8000/admin

# Test specific technique
./target/release/byps test http://127.0.0.1:8000/admin --technique path_bypass

# List available techniques
./target/release/byps list
```

## Testing Infrastructure

Python mock servers for localhost testing:

```bash
# Basic path bypass server (port 8000)
python3 tests/fixtures/mock_servers/basic_server.py

# Header bypass server (port 8001)
python3 tests/fixtures/mock_servers/header_server.py

# Mock WAF server (port 8002)
python3 tests/fixtures/mock_servers/waf_server.py
```

## Architecture

```
Rust CLI Layer (arg parsing, output) 
    ↓ FFI
C++ Core Engine (bypass techniques, network, WAF detection)
```

## Implementation Status

- [x] Project structure and build system
- [x] Common utilities (logger, error handling, utils)
- [x] Path bypass techniques
- [x] Header forgery techniques
- [x] Encoding techniques
- [x] Protocol abuse basics
- [x] C API / FFI bridge
- [x] Rust CLI interface
- [x] Python mock test servers
- [x] JSON output formatter
- [x] Terminal output formatter
- [ ] Network layer (HTTP client)
- [ ] WAF detection engine
- [ ] Request smuggling
- [ ] Advanced timing techniques
- [ ] Complete test suite
- [ ] Documentation

## Contributing

See [Developer Documentation](./Developer/) for contributing guidelines.

## License

MIT License - see LICENSE file

---

**Made with ❤️ for the security community**
