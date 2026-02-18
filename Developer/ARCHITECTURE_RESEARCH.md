# Architecture Research Notes

## System Overview

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Rust CLI Layer                          │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │ Argument    │  │ Config       │  │ Output           │  │
│  │ Parsing     │  │ Management   │  │ Formatting       │  │
│  │ (clap)      │  │ (YAML)       │  │ (JSON/CSV/HTML)  │  │
│  └─────────────┘  └──────────────┘  └──────────────────┘  │
│         │                │                    │             │
│         └────────────────┴────────────────────┘             │
│                          │                                  │
│                    ┌─────▼─────┐                           │
│                    │ FFI Bridge │                           │
│                    └─────┬─────┘                           │
└──────────────────────────┼──────────────────────────────────┘
                           │ (extern "C" interface)
┌──────────────────────────▼──────────────────────────────────┐
│                  C++ Core Engine                            │
│  ┌────────────────────────────────────────────────────┐    │
│  │              Bypass Engine                          │    │
│  │  (Orchestration, Strategy, Result Aggregation)     │    │
│  └───┬────────────────────────────────────────────┬───┘    │
│      │                                            │         │
│  ┌───▼─────────────┐                   ┌─────────▼──────┐  │
│  │ Technique Layer │                   │ Scanner Layer  │  │
│  │ ┌─────────────┐ │                   │ ┌────────────┐ │  │
│  │ │Path Bypass  │ │                   │ │ Strategies │ │  │
│  │ │Header Forge │ │                   │ │ Results    │ │  │
│  │ │Protocol     │ │                   │ └────────────┘ │  │
│  │ │Encoding     │ │                   └────────────────┘  │
│  │ │Timing       │ │                                       │
│  │ │Smuggling    │ │                                       │
│  │ │Unicode      │ │                                       │
│  │ └─────────────┘ │                                       │
│  └────────┬────────┘                                       │
│           │                                                │
│  ┌────────▼────────────────────────────────────────────┐   │
│  │              Network Layer                          │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │   │
│  │  │ Socket   │  │ HTTP     │  │ TLS/SSL          │  │   │
│  │  │ TCP/UDP  │  │ Parser   │  │ HTTP/2           │  │   │
│  │  └──────────┘  └──────────┘  └──────────────────┘  │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                            │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              WAF Detection Layer                    │   │
│  │  ┌─────────────┐  ┌──────────┐  ┌──────────────┐   │   │
│  │  │Fingerprint  │  │ Detector │  │ Evasion      │   │   │
│  │  └─────────────┘  └──────────┘  └──────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                            │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              Common Utilities                        │   │
│  │  Logger │ Error Handler │ Utils │ Types              │   │
│  └─────────────────────────────────────────────────────┘   │
└────────────────────────────────────────────────────────────┘
```

## Component Responsibilities

### 1. Rust CLI Layer

**Purpose**: User interface and system integration

**Responsibilities**:
- Command-line argument parsing and validation
- Configuration file loading and merging
- Calling C++ core via FFI
- Output formatting and presentation
- Error handling and user feedback

**Key Design Decisions**:
- Use Rust for memory safety in user-facing layer
- Leverage clap for robust CLI parsing
- Minimal business logic (delegate to C++)
- Type-safe FFI conversions

### 2. FFI Bridge

**Purpose**: Safe interface between Rust and C++

**Responsibilities**:
- Expose C API from C++ core
- Convert between Rust and C types
- Handle error propagation
- Manage memory across FFI boundary
- Thread safety guarantees

**Key Design Decisions**:
- extern "C" interface to avoid name mangling
- Opaque pointers for complex C++ objects
- Error codes for error propagation
- Caller manages memory (Rust owns strings passed in)

**Interface Example**:
```c
// C API
typedef struct BypsEngine BypsEngine;

BypsEngine* byps_engine_new();
int byps_engine_scan(BypsEngine* engine, 
                     const char* url, 
                     const char* config_json,
                     char** result_json);
void byps_engine_free(BypsEngine* engine);
void byps_string_free(char* str);
```

### 3. C++ Core Engine

**Purpose**: Main orchestration and business logic

**Responsibilities**:
- Coordinate technique execution
- Manage execution strategies
- Aggregate results
- Handle retries and fallbacks
- Parallel execution management

**Key Design Decisions**:
- Strategy pattern for scanning modes
- Observer pattern for progress reporting
- Factory pattern for technique creation
- Thread pool for parallel execution

**Class Design**:
```cpp
class BypassEngine {
public:
    BypassEngine(const Config& config);
    ScanResult scan(const std::string& url);
    void setStrategy(std::unique_ptr<ScanStrategy> strategy);
    
private:
    std::vector<std::unique_ptr<Technique>> techniques_;
    std::unique_ptr<ScanStrategy> strategy_;
    WAFDetector waf_detector_;
    ThreadPool thread_pool_;
    ResultAggregator result_aggregator_;
};
```

### 4. Technique Layer

**Purpose**: Implement bypass techniques

**Responsibilities**:
- Generate bypass variations
- Apply technique-specific logic
- Validate generated payloads
- Report success/failure

**Key Design Decisions**:
- Base Technique interface for polymorphism
- Each technique is independent
- Composable techniques (chain multiple)
- Configurable parameters per technique

**Interface**:
```cpp
class Technique {
public:
    virtual ~Technique() = default;
    virtual std::string name() const = 0;
    virtual std::vector<Request> generate(const Request& base) = 0;
    virtual bool isApplicable(const Response& response) const = 0;
};

class PathBypass : public Technique {
public:
    std::string name() const override { return "path_bypass"; }
    std::vector<Request> generate(const Request& base) override;
    bool isApplicable(const Response& response) const override;
};
```

### 5. Network Layer

**Purpose**: Low-level network communication

**Responsibilities**:
- TCP/UDP socket operations
- HTTP protocol handling
- TLS/SSL encryption
- HTTP/2 support
- Connection pooling
- Timeout management

**Key Design Decisions**:
- RAII for resource management
- Non-blocking I/O with timeouts
- OpenSSL for TLS
- nghttp2 for HTTP/2
- Connection reuse for performance

**Architecture**:
```cpp
class Socket {
    int fd_;
    SocketType type_;
    bool blocking_;
public:
    Socket(SocketType type);
    ~Socket(); // RAII cleanup
    void connect(const std::string& host, uint16_t port, 
                 std::chrono::milliseconds timeout);
    size_t send(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recv(size_t max_size);
};

class HTTPClient {
    ConnectionPool pool_;
    TLSContext tls_ctx_;
public:
    Response send(const Request& req);
};
```

### 6. WAF Detection Layer

**Purpose**: Identify and adapt to WAF presence

**Responsibilities**:
- Fingerprint WAF type
- Analyze responses for WAF signatures
- Calculate confidence scores
- Suggest appropriate techniques
- Adapt to WAF behavior

**Key Design Decisions**:
- Signature-based detection
- Statistical analysis for confidence
- Passive and active detection
- WAF-specific technique mapping

**Flow**:
```
1. Send probe request
2. Analyze response (headers, body, timing)
3. Compare against signature database
4. Calculate confidence score
5. Select appropriate evasion techniques
```

### 7. Scanner Layer

**Purpose**: Define scanning strategies

**Responsibilities**:
- Implement strategy pattern
- Control execution flow
- Manage resources (threads, connections)
- Progress tracking
- Result management

**Strategies**:
```cpp
class ScanStrategy {
public:
    virtual ~ScanStrategy() = default;
    virtual ScanResult execute(const std::string& url,
                              std::vector<Technique*>& techniques) = 0;
};

class FastStrategy : public ScanStrategy {
    // Try common techniques only, parallel execution
};

class ThoroughStrategy : public ScanStrategy {
    // Try all techniques, all variations, slower
};

class StealthStrategy : public ScanStrategy {
    // Slow, random delays, sequential, human-like
};
```

## Design Patterns

### 1. Strategy Pattern
**Used in**: Scanning strategies, technique selection

**Benefits**:
- Easy to add new strategies
- Runtime strategy switching
- Encapsulates algorithm variations

### 2. Factory Pattern
**Used in**: Technique creation, output formatter creation

**Benefits**:
- Centralized object creation
- Easy to extend with new types
- Decouples creation from usage

### 3. Observer Pattern
**Used in**: Progress reporting, event logging

**Benefits**:
- Loose coupling
- Multiple observers
- Real-time updates

### 4. Builder Pattern
**Used in**: Request building, configuration building

**Benefits**:
- Fluent interface
- Step-by-step construction
- Validation at build time

### 5. Template Method Pattern
**Used in**: Technique base class

**Benefits**:
- Common algorithm structure
- Customization points
- Code reuse

## Threading Model

### Thread Pool
**Size**: Configurable (default: CPU cores)

**Work Distribution**:
1. Main thread: orchestration
2. Worker threads: technique execution
3. I/O threads: network operations

**Synchronization**:
- Mutex for shared state
- Lock-free queues for work distribution
- Atomic counters for statistics

**Thread Safety**:
- Immutable Request/Response objects
- Thread-local storage for per-thread state
- No shared mutable state

## Memory Management

### C++ Layer
**Approach**: RAII (Resource Acquisition Is Initialization)

**Guidelines**:
- Smart pointers (unique_ptr, shared_ptr)
- Stack allocation when possible
- Explicit ownership
- No manual delete
- Exception-safe

### FFI Boundary
**Approach**: Explicit ownership transfer

**Rules**:
1. Rust allocates strings passed to C++
2. C++ copies strings, Rust frees
3. C++ allocates result strings
4. Rust frees results via free function
5. Opaque pointers for complex objects

### Example:
```cpp
// C++ allocates, Rust must free
char* byps_engine_scan(...) {
    std::string result = scan_impl();
    return strdup(result.c_str()); // malloc'd
}

// Rust frees
void byps_string_free(char* str) {
    free(str);
}
```

## Error Handling

### C++ Layer
**Approach**: Exceptions + Error codes

**Exception Types**:
```cpp
class BypsException : public std::exception {
    ErrorCode code_;
    std::string message_;
    std::string stack_trace_;
};

enum class ErrorCode {
    SUCCESS = 0,
    NETWORK_ERROR = 1,
    PARSE_ERROR = 2,
    TIMEOUT = 3,
    WAF_DETECTED = 4,
    // ...
};
```

**Exception Boundaries**:
- Exceptions within C++ only
- Convert to error codes at FFI boundary
- Log all exceptions

### FFI Layer
**Approach**: Error codes

**Convention**:
- 0 = success
- Negative = error
- Positive = warning

### Rust Layer
**Approach**: Result<T, E>

**Error Type**:
```rust
#[derive(Debug)]
enum BypsError {
    NetworkError(String),
    ParseError(String),
    Timeout,
    WafDetected,
    FfiError(i32),
}

impl std::error::Error for BypsError {}
```

## Configuration Management

### Configuration Hierarchy
1. Built-in defaults
2. System config file (/etc/byps/config.yaml)
3. User config file (~/.config/byps/config.yaml)
4. Project config file (./byps.yaml)
5. Command-line arguments

**Merging**: Later sources override earlier ones

### Configuration Structure
```yaml
# config.yaml
timeout: 10  # seconds
retries: 3
concurrent: 10
verbosity: info

techniques:
  - path_bypass
  - header_forge
  - encoding

strategy: balanced  # fast | balanced | thorough | stealth

output:
  format: json
  file: results.json
  
waf:
  detect: true
  adapt: true
```

## Performance Considerations

### Optimization Strategies

1. **Connection Reuse**
   - Keep-alive connections
   - Connection pooling
   - HTTP/2 multiplexing

2. **Parallel Execution**
   - Thread pool for techniques
   - Async I/O for network
   - Batch request processing

3. **Caching**
   - DNS resolution cache
   - Technique result cache
   - WAF detection cache

4. **Memory Efficiency**
   - String view for parsing (no copies)
   - Move semantics
   - Object pooling for frequent allocations

5. **Compilation Optimizations**
   - -O3 optimization level
   - Link-time optimization (LTO)
   - Profile-guided optimization (PGO)

### Performance Targets
- **Throughput**: > 1000 requests/second
- **Latency**: < 100ms per request (network excluded)
- **Memory**: < 100MB for typical scan
- **Startup**: < 100ms

## Security Considerations

### Input Validation
- Validate all user inputs
- Sanitize URLs
- Check file paths
- Validate configuration

### Resource Limits
- Maximum concurrent connections
- Request timeout limits
- Response size limits
- Memory usage limits

### Privilege Management
- Drop privileges after raw socket operations
- Minimal required permissions
- Secure file handling

### Secure Coding
- No buffer overflows (use std::string, std::vector)
- Bounds checking
- Integer overflow protection
- No use-after-free (RAII)

## Extensibility

### Adding New Techniques

1. Create new class inheriting from Technique
2. Implement required methods
3. Register in TechniqueFactory
4. Add tests
5. Update documentation

**Example**:
```cpp
class CustomTechnique : public Technique {
public:
    std::string name() const override {
        return "custom_technique";
    }
    
    std::vector<Request> generate(const Request& base) override {
        // Implementation
    }
};

// Register
TechniqueFactory::registerTechnique("custom_technique",
    []() { return std::make_unique<CustomTechnique>(); });
```

### Plugin System (Future)
- Dynamic library loading
- Plugin interface
- Sandboxed execution
- Version compatibility

## Testing Architecture

### Test Layers
1. **Unit Tests**: Individual components
2. **Integration Tests**: Component interactions
3. **System Tests**: Full system with mock servers
4. **Performance Tests**: Benchmarks

### Mock Objects
- Mock HTTP server (Python)
- Mock WAF (Python)
- Mock network layer (C++)
- Mock techniques (for engine testing)

### Test Doubles
- Stubs: Simple responses
- Mocks: Verify interactions
- Fakes: Working implementations
- Spies: Record calls

## Logging and Debugging

### Log Levels
- TRACE: Very detailed
- DEBUG: Detailed
- INFO: General
- WARN: Warnings
- ERROR: Errors

### Log Format
```
[2024-01-15 10:30:45.123] [INFO] [engine.cpp:45] Starting scan of http://example.com
```

### Debug Features
- Request/response logging
- Technique execution trace
- Performance profiling
- Memory usage tracking

## Documentation Architecture

### Code Documentation
- Doxygen for C++
- Rustdoc for Rust
- Inline comments for complex logic

### User Documentation
- README: Quick start
- ARCHITECTURE: This document
- TECHNIQUES: Technique explanations
- API: API reference
- CONTRIBUTING: Contribution guide

## Deployment Architecture

### Build Artifacts
1. Static binary (Linux, macOS, Windows)
2. Shared library (for embedding)
3. Docker image
4. Package manager packages

### Dependencies
**Required**:
- C++17 compiler
- CMake 3.15+
- Boost 1.70+
- OpenSSL 1.1+

**Optional**:
- nghttp2 (HTTP/2 support)
- jemalloc (better memory allocation)

## Future Enhancements

### Planned Features
1. **Machine Learning**
   - Automatic technique selection
   - Success prediction
   - Pattern recognition

2. **Distributed Scanning**
   - Master-worker architecture
   - Load distribution
   - Result aggregation

3. **GUI Interface**
   - Web-based dashboard
   - Real-time visualization
   - Interactive reports

4. **Cloud Integration**
   - Cloud-based scanning
   - Distributed results storage
   - API service

### Scalability Considerations
- Horizontal scaling (multiple instances)
- Rate limiting coordination
- Shared result storage
- Load balancing

## Conclusion

This architecture provides a solid foundation for a high-performance, extensible CLI bypass tool. The separation of concerns between Rust and C++ layers, combined with well-defined interfaces and design patterns, ensures maintainability and allows for future enhancements.
