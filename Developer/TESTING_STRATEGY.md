# Testing Strategy - CLI Bypass Tool

## Overview
This document outlines the comprehensive testing strategy for the CLI bypass tool, with special emphasis on using Python-based localhost servers due to sandbox limitations.

## Testing Environment

### Sandbox Limitations
**CRITICAL**: Due to sandbox environment constraints:
- No external network access available
- No access to real WAF services
- No access to production websites
- Must use localhost-only testing

### Solution: Python Localhost Server
All testing will be conducted using **Python HTTP servers running on localhost**:
- Simple Python HTTP server for basic tests
- Flask/FastAPI applications for advanced scenarios
- Mock WAF implementations in Python
- Simulated authentication and authorization

## Testing Pyramid

### Level 1: Unit Tests (70% of tests)

#### C++ Unit Tests (GoogleTest)
**Location**: `tests/cpp/`

**Components to Test**:

1. **Common Utilities** (`test_common.cpp`)
   - Logger functionality
   - Error handling
   - String utilities
   - Encoding/decoding functions
   - URL parsing

2. **HTTP Parser** (`test_parser.cpp`)
   - Request parsing
   - Response parsing
   - Header extraction
   - Chunked encoding
   - Malformed input handling
   - Edge cases (empty headers, duplicate headers, etc.)

3. **Techniques** (`test_techniques.cpp`)
   - Path manipulation generation
   - Header forgery generation
   - Encoding variations
   - Protocol abuse payloads
   - Unicode normalization
   - Payload obfuscation

4. **Network Layer** (`test_network.cpp`)
   - Socket operations (mocked)
   - Connection handling
   - Timeout mechanisms
   - TLS initialization
   - Error handling

5. **Engine** (`test_engine.cpp`)
   - Technique orchestration
   - Priority management
   - Result aggregation
   - Strategy selection

**Test Characteristics**:
- Fast execution (< 1 second per test)
- No network I/O
- Use mocks for external dependencies
- Isolated (no shared state)
- Deterministic results

**Example Test**:
```cpp
TEST(PathBypassTest, GenerateEncodedVariations) {
    PathBypass pb;
    auto variations = pb.generateVariations("/admin");
    
    EXPECT_TRUE(std::find(variations.begin(), variations.end(), 
                         "/admin/") != variations.end());
    EXPECT_TRUE(std::find(variations.begin(), variations.end(), 
                         "/%61dmin") != variations.end());
    EXPECT_TRUE(std::find(variations.begin(), variations.end(), 
                         "/ADMIN") != variations.end());
}
```

#### Rust Unit Tests (cargo test)
**Location**: `src/` (inline tests)

**Components to Test**:
1. CLI argument parsing
2. Configuration loading
3. Output formatting
4. FFI bridge (error conversion)
5. Type conversions

**Test Characteristics**:
- Use `#[cfg(test)]` modules
- Test public API
- Mock C++ FFI calls

### Level 2: Integration Tests (20% of tests)

#### Python Mock Server Setup
**Location**: `tests/fixtures/mock_servers/`

**Server Types**:

1. **Basic HTTP Server** (`basic_server.py`)
```python
#!/usr/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
import json

class TestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # Simulate 403 for /admin
        if self.path == '/admin':
            self.send_response(403)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            self.wfile.write(b'Forbidden')
            return
        
        # Allow /admin/ (path bypass simulation)
        if self.path == '/admin/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            self.wfile.write(b'Admin page accessed!')
            return
        
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(b'OK')

if __name__ == '__main__':
    server = HTTPServer(('127.0.0.1', 8000), TestHandler)
    print('Server running on http://127.0.0.1:8000')
    server.serve_forever()
```

2. **Header-Sensitive Server** (`header_server.py`)
```python
#!/usr/bin/env python3
from flask import Flask, request

app = Flask(__name__)

@app.route('/admin')
def admin():
    # Check X-Forwarded-For header
    if request.headers.get('X-Forwarded-For') == '127.0.0.1':
        return 'Admin access granted', 200
    
    # Check X-Original-URL header
    if request.headers.get('X-Original-URL') == '/allowed':
        return 'Admin access granted', 200
    
    return 'Forbidden', 403

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8001)
```

3. **Mock WAF Server** (`waf_server.py`)
```python
#!/usr/bin/env python3
from flask import Flask, request, Response

app = Flask(__name__)

class SimpleWAF:
    def __init__(self):
        self.blocked_patterns = [
            '/admin',
            '../',
            '<script>',
            'union select'
        ]
    
    def check_request(self, path, headers):
        # Simple pattern matching
        for pattern in self.blocked_patterns:
            if pattern.lower() in path.lower():
                return False
        return True

waf = SimpleWAF()

@app.before_request
def waf_check():
    if not waf.check_request(request.path, request.headers):
        return Response('Blocked by WAF', status=403, 
                       headers={'Server': 'MockWAF/1.0'})

@app.route('/<path:path>')
def catch_all(path):
    return f'Path accessed: {path}', 200

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8002)
```

4. **Protocol-Sensitive Server** (`protocol_server.py`)
```python
#!/usr/bin/env python3
from flask import Flask, request

app = Flask(__name__)

@app.route('/test')
def test():
    # Check HTTP method
    if request.method == 'GETT':  # Custom method
        return 'Custom method accepted', 200
    
    # Check for duplicate headers
    if len(request.headers.getlist('Host')) > 1:
        return 'Duplicate headers detected', 200
    
    return 'Normal response', 200

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8003)
```

5. **Encoding-Sensitive Server** (`encoding_server.py`)
```python
#!/usr/bin/env python3
from flask import Flask, request
from urllib.parse import unquote

app = Flask(__name__)

@app.route('/<path:path>')
def check_encoding(path):
    # Simulate different decoding levels
    decoded_once = unquote(path)
    decoded_twice = unquote(decoded_once)
    
    # Block "admin" in first decode
    if 'admin' in decoded_once.lower():
        return 'Blocked', 403
    
    # But allow in second decode (double encoding bypass)
    if 'admin' in decoded_twice.lower():
        return 'Access granted via double encoding', 200
    
    return 'OK', 200

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8004)
```

#### Integration Test Execution
**Location**: `tests/integration/`

**Test Flow**:
1. Start Python mock server in background
2. Run bypass tool against localhost server
3. Verify expected bypass success/failure
4. Capture and validate results
5. Stop mock server

**Example Integration Test** (`test_bypass.rs`):
```rust
#[test]
fn test_path_bypass_trailing_slash() {
    // Start mock server
    let server = start_mock_server("basic_server.py", 8000);
    
    // Run bypass tool
    let result = run_bypass("http://127.0.0.1:8000/admin", 
                           vec!["path_bypass"]);
    
    // Verify results
    assert!(result.is_ok());
    assert!(result.unwrap().success);
    assert_eq!(result.unwrap().technique_used, "trailing_slash");
    
    // Cleanup
    server.stop();
}
```

#### Test Categories:

1. **Path Bypass Tests**
   - Trailing slash bypass
   - URL encoding bypass
   - Case variation bypass
   - Null byte bypass
   - Path traversal bypass

2. **Header Bypass Tests**
   - X-Forwarded-For bypass
   - X-Original-URL bypass
   - Host header bypass
   - Custom header bypass

3. **Protocol Bypass Tests**
   - HTTP version variation
   - Custom method bypass
   - Duplicate header bypass
   - Content-Length manipulation

4. **Encoding Bypass Tests**
   - Single URL encoding
   - Double URL encoding
   - Triple URL encoding
   - Unicode normalization
   - Mixed encoding

5. **WAF Detection Tests**
   - WAF fingerprinting accuracy
   - WAF type identification
   - Confidence scoring
   - False positive rate

### Level 3: End-to-End Tests (10% of tests)

#### E2E Test Scenarios
**Location**: `tests/e2e/`

**Scenarios**:

1. **Complete Bypass Workflow**
   - Start with blocked URL
   - Detect WAF
   - Apply multiple techniques
   - Find successful bypass
   - Generate report

2. **Stealth Mode Testing**
   - Slow request rate
   - Random delays
   - User agent rotation
   - Connection reuse

3. **Aggressive Mode Testing**
   - High concurrency
   - All techniques enabled
   - Fast execution
   - Comprehensive coverage

4. **Output Format Testing**
   - JSON output validation
   - CSV output validation
   - HTML report generation
   - Terminal output formatting

## Performance Testing

### Benchmarks
**Location**: `benchmarks/`

**Metrics to Measure**:
1. Requests per second
2. Technique generation speed
3. Parser performance
4. Memory usage
5. CPU utilization
6. Connection establishment time

**Benchmark Tests**:
```cpp
// bench_techniques.cpp
BENCHMARK(BM_PathGeneration) {
    PathBypass pb;
    for (auto _ : state) {
        auto variations = pb.generateVariations("/admin");
        benchmark::DoNotOptimize(variations);
    }
}
```

**Performance Targets**:
- Request throughput: > 1000 req/sec
- Technique generation: < 1ms per technique
- Memory usage: < 100MB for typical scan
- Startup time: < 100ms

## Test Data and Fixtures

### Test Fixtures
**Location**: `tests/fixtures/`

**Contents**:
1. **Sample HTTP Responses** (`sample_responses/`)
   - 200 OK responses
   - 401 Unauthorized responses
   - 403 Forbidden responses
   - 404 Not Found responses
   - WAF block pages (various WAFs)
   - Malformed responses

2. **Test Payloads** (`test_payloads/`)
   - Known bypass payloads
   - Encoding variations
   - Unicode samples
   - Smuggling templates

3. **Configuration Files** (`test_configs/`)
   - Test-specific configs
   - Mock WAF configs
   - Performance test configs

## Continuous Testing

### Pre-commit Tests
- Fast unit tests only
- Code formatting check
- Basic syntax validation

### CI Pipeline Tests
**On Pull Request**:
1. All unit tests
2. Integration tests (with mock servers)
3. Code coverage check (minimum 80%)
4. Static analysis (clang-tidy, clippy)
5. Memory leak detection
6. Performance regression tests

**On Merge to Main**:
1. Full test suite
2. E2E tests
3. Performance benchmarks
4. Documentation build
5. Release binary build

### Nightly Tests
- Extended integration tests
- Long-running performance tests
- Memory profiling
- Fuzz testing

## Test Coverage

### Coverage Goals
- **Overall**: 80% code coverage minimum
- **Core engine**: 90% coverage
- **Techniques**: 85% coverage
- **Network layer**: 75% coverage (due to platform-specific code)
- **CLI layer**: 80% coverage

### Coverage Tools
- C++: gcov/lcov
- Rust: tarpaulin
- Combined reporting with codecov.io

### Coverage Reports
```bash
# Generate C++ coverage
cd build
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make coverage
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# Generate Rust coverage
cargo tarpaulin --out Html --output-dir target/coverage
```

## Mock Server Management

### Server Lifecycle

**Automated Server Management** (`tests/lib/server_manager.py`):
```python
#!/usr/bin/env python3
import subprocess
import time
import requests
import signal

class MockServerManager:
    def __init__(self):
        self.servers = {}
    
    def start_server(self, script_name, port):
        proc = subprocess.Popen(['python3', f'mock_servers/{script_name}'],
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
        self.servers[port] = proc
        
        # Wait for server to be ready
        for _ in range(30):  # 3 seconds timeout
            try:
                requests.get(f'http://127.0.0.1:{port}/', timeout=0.1)
                return True
            except:
                time.sleep(0.1)
        
        return False
    
    def stop_server(self, port):
        if port in self.servers:
            self.servers[port].send_signal(signal.SIGTERM)
            self.servers[port].wait(timeout=5)
            del self.servers[port]
    
    def stop_all(self):
        for port in list(self.servers.keys()):
            self.stop_server(port)
```

### Test Server Requirements

**Python Dependencies** (`tests/requirements.txt`):
```
flask>=2.0.0
requests>=2.26.0
pytest>=6.2.0
```

**Installation**:
```bash
pip3 install -r tests/requirements.txt
```

## Testing Best Practices

### 1. Test Isolation
- Each test should be independent
- Clean up resources after each test
- Use unique ports for each test server
- Reset state between tests

### 2. Test Reliability
- Avoid race conditions
- Use proper timeouts
- Handle network errors gracefully
- Retry flaky tests with exponential backoff

### 3. Test Maintainability
- Clear test names describing what is tested
- Use helper functions to reduce duplication
- Keep tests simple and focused
- Document complex test scenarios

### 4. Test Performance
- Fast unit tests (< 1s each)
- Reasonable integration tests (< 10s each)
- Optimize test execution order
- Parallelize independent tests

## Security Testing

### Vulnerability Scanning
1. **Static Analysis**
   - clang-tidy for C++
   - clippy for Rust
   - cppcheck
   - Include-what-you-use

2. **Dynamic Analysis**
   - Valgrind for memory leaks
   - AddressSanitizer
   - UndefinedBehaviorSanitizer
   - ThreadSanitizer

3. **Dependency Scanning**
   - Check for known vulnerabilities in dependencies
   - Regular dependency updates
   - SBOM (Software Bill of Materials) generation

### Fuzzing
**Location**: `tests/fuzz/`

**Fuzz Targets**:
- HTTP parser
- URL parser
- Encoding functions
- Request builder

**Tools**:
- AFL (American Fuzzy Lop)
- libFuzzer
- cargo-fuzz for Rust

## Test Documentation

### Test Plan Document
For each major feature:
1. Feature description
2. Test objectives
3. Test cases
4. Expected results
5. Pass/fail criteria

### Test Reports
After each test run:
1. Test summary (passed/failed/skipped)
2. Coverage report
3. Performance metrics
4. Failed test details
5. Regression analysis

## Localhost Testing Guidelines

### General Principles
✅ **DO**:
- Use localhost (127.0.0.1) for all tests
- Use Python for mock servers
- Test against simulated WAF behavior
- Verify bypass logic, not actual bypasses
- Focus on technique correctness
- Test error handling thoroughly

❌ **DON'T**:
- Attempt external network connections
- Test against real WAFs
- Test against production sites
- Assume network availability
- Rely on internet services
- Use hardcoded IP addresses other than localhost

### Sample Test Commands

**Start Mock Server**:
```bash
python3 tests/fixtures/mock_servers/basic_server.py &
SERVER_PID=$!
```

**Run Tests**:
```bash
# Run specific test
./build/bin/byps --url http://127.0.0.1:8000/admin \
                 --technique path_bypass \
                 --output json

# Run test suite
cargo test
cd build && ctest
```

**Cleanup**:
```bash
kill $SERVER_PID
```

## Success Criteria

A test suite is considered successful when:
- [ ] All unit tests pass
- [ ] All integration tests pass with mock servers
- [ ] Code coverage >= 80%
- [ ] No memory leaks detected
- [ ] Performance benchmarks met
- [ ] All security scans pass
- [ ] Documentation tests pass
- [ ] E2E scenarios complete successfully

## Continuous Improvement

### Test Metrics to Track
1. Test execution time trends
2. Code coverage trends
3. Flaky test rate
4. Bug escape rate
5. Performance regression rate

### Regular Reviews
- Monthly test suite audit
- Quarterly performance review
- Remove obsolete tests
- Add tests for new bugs
- Update mock servers for new techniques

## Conclusion

This testing strategy ensures that the CLI bypass tool is thoroughly tested despite sandbox limitations. By using Python localhost servers, we can simulate various WAF behaviors and validate all bypass techniques in a controlled, safe environment. The comprehensive test pyramid approach ensures quality at all levels, from unit tests to end-to-end scenarios.
