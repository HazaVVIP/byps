#ifndef BYPS_COMMON_TYPES_HPP
#define BYPS_COMMON_TYPES_HPP

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>

namespace byps {

// HTTP method types
enum class HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    PATCH,
    TRACE,
    CONNECT,
    CUSTOM
};

// WAF types
enum class WAFType {
    UNKNOWN,
    CLOUDFLARE,
    AKAMAI,
    AWS_WAF,
    IMPERVA,
    F5_BIG_IP,
    MOD_SECURITY,
    SUCURI,
    STACK_PATH
};

// Request configuration
struct RequestConfig {
    std::string url;
    int timeout_ms;
    int max_retries;
    bool follow_redirects;
    std::map<std::string, std::string> headers;
    
    RequestConfig() 
        : timeout_ms(10000), max_retries(3), follow_redirects(true) {}
};

// Bypass result
struct BypassResult {
    bool success;
    int status_code;
    std::string technique_used;
    double response_time_ms;
    std::string response_body;
    std::map<std::string, std::string> response_headers;
    std::string error_message;
    
    BypassResult() 
        : success(false), status_code(0), response_time_ms(0.0) {}
};

// WAF information
struct WAFInfo {
    WAFType type;
    float confidence;
    std::string version;
    std::vector<std::string> detected_headers;
    
    WAFInfo() : type(WAFType::UNKNOWN), confidence(0.0f) {}
};

// HTTP request structure
struct HTTPRequest {
    HTTPMethod method;
    std::string custom_method;
    std::string url;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HTTPRequest() : method(HTTPMethod::GET), version("HTTP/1.1") {}
};

// HTTP response structure
struct HTTPResponse {
    int status_code;
    std::string status_message;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    double response_time_ms;
    
    HTTPResponse() : status_code(0), response_time_ms(0.0) {}
};

// Scan result aggregation
struct ScanResult {
    std::string target_url;
    bool bypass_successful;
    std::vector<BypassResult> attempts;
    WAFInfo waf_info;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    
    ScanResult() : bypass_successful(false) {}
};

} // namespace byps

#endif // BYPS_COMMON_TYPES_HPP
