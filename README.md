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

### CLI Modes
- **Scan Mode**: Generate bypass variations for manual testing
- **Test Mode**: Test a specific bypass technique
- **Exploit Mode** ⭐NEW: Automated exploitation with live testing
- **Wizard Mode** ⭐NEW: Interactive guided testing for beginners
- **Detect Mode**: WAF fingerprinting and detection
- **List Mode**: Display all available techniques

### Enhanced Help System
Comprehensive help text with:
- Detailed command descriptions
- Strategy explanations (fast, balanced, thorough, stealth)
- Technique documentation
- Real-world usage examples
- Output format guidance

```bash
# Access enhanced help
byps --help              # Main help with all features
byps scan --help         # Scan-specific help with examples
byps exploit --help      # Exploit mode documentation
byps wizard --help       # Wizard mode guide
```

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

### Installation

The easiest way to install Byps is using the automated installer script:

```bash
wget https://raw.githubusercontent.com/HazaVVIP/byps/main/install.sh
chmod +x install.sh
./install.sh
```

The installer will:
- ✅ Automatically detect your operating system
- ✅ Install all required dependencies (CMake, C++ compiler, OpenSSL, etc.)
- ✅ Install Rust if not present
- ✅ Clone and build the Byps project
- ✅ Install the `byps` command globally

**Supported Operating Systems:**
- Ubuntu/Debian/Linux Mint
- Fedora/RHEL/CentOS
- Arch Linux/Manjaro
- macOS (requires Homebrew)

**Uninstall:**
```bash
./install.sh --uninstall
```

### Usage

```bash
# Display help
byps -h           # Short help
byps --help       # Detailed help with examples

# List available bypass techniques
byps list

# Test a specific technique
byps test http://example.com/admin --technique path_bypass

# Full scan with all techniques
byps scan http://example.com/admin

# Automated exploitation mode (NEW!)
byps exploit http://example.com/admin

# Interactive wizard mode (NEW!)
byps wizard

# Save results to file
byps scan http://example.com/admin -o results.html --output html

# Use stealth mode
byps scan http://example.com/admin --strategy stealth

# Verbose output
byps scan http://example.com/admin -v --output terminal
```

## 🎯 New Features

### Exploit Mode
Automated exploitation mode that combines scanning with live testing:

```bash
# Basic exploitation
byps exploit http://example.com/admin

# Fast exploitation with specific techniques
byps exploit http://example.com/api -t path_bypass,header_forge -s fast

# Thorough exploitation with results saved
byps exploit http://example.com/secret -s thorough -o exploits.json

# Stealth exploitation with limited tests
byps exploit http://example.com/admin -s stealth -m 20 -v
```

**Features:**
- Automatically generates and tests bypass variations
- Reports successful bypasses with visual indicators
- Configurable test limits and strategies
- Saves working exploits to file
- Beautiful colored output with progress tracking

### Wizard Mode
Interactive guided mode perfect for beginners:

```bash
# Start the interactive wizard
byps wizard

# Wizard with verbose output
byps wizard -v
```

**Wizard Process:**
1. Enter target URL
2. Select mode (scan or exploit)
3. Choose bypass techniques
4. Select scan strategy
5. Optional file output
6. Review and confirm settings
7. Execute and view results

The wizard provides a user-friendly interface with step-by-step guidance through the entire bypass testing process.

## 📖 Examples

### Basic Scan
```bash
$ byps scan http://127.0.0.1:8000/admin -v --output terminal

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
$ byps test http://example.com/secret --technique url_encoding -o results.json

# Output: JSON with all encoding variations
```

### HTML Report
```bash
$ byps scan http://example.com/api --output html -o report.html

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
byps scan http://127.0.0.1:8000/admin
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
