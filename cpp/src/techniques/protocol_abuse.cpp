#include "techniques/protocol_abuse.hpp"
#include <algorithm>

namespace byps {

ProtocolAbuse::ProtocolAbuse() {
    http_versions_ = {"HTTP/0.9", "HTTP/1.0", "HTTP/1.1", "HTTP/2.0"};
    custom_methods_ = {"GETT", "POSTX", "PUTT", "DELETEX", "get", "Get", "GET "};
}

std::vector<std::string> ProtocolAbuse::getHttpVersions() {
    return http_versions_;
}

std::vector<std::string> ProtocolAbuse::getMethodVariations(const std::string& base_method) {
    std::vector<std::string> variations;
    
    // Exact method
    variations.push_back(base_method);
    
    // Lowercase
    std::string lower = base_method;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    variations.push_back(lower);
    
    // Uppercase
    std::string upper = base_method;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    variations.push_back(upper);
    
    // Mixed case
    if (!base_method.empty()) {
        std::string mixed = lower;
        mixed[0] = std::toupper(mixed[0]);
        variations.push_back(mixed);
    }
    
    // With trailing space
    variations.push_back(base_method + " ");
    
    // With extra character
    variations.push_back(base_method + "X");
    
    return variations;
}

std::vector<std::string> ProtocolAbuse::getCustomMethods() {
    return custom_methods_;
}

HTTPRequest ProtocolAbuse::addDuplicateHeaders(const HTTPRequest& req) {
    HTTPRequest modified = req;
    
    // Add duplicate Host header
    if (modified.headers.find("Host") != modified.headers.end()) {
        modified.headers.insert({"Host", "localhost"});
    }
    
    return modified;
}

HTTPRequest ProtocolAbuse::addConflictingHeaders(const HTTPRequest& req) {
    HTTPRequest modified = req;
    
    // Add conflicting Content-Length and Transfer-Encoding
    modified.headers["Content-Length"] = "10";
    modified.headers["Transfer-Encoding"] = "chunked";
    
    return modified;
}

HTTPRequest ProtocolAbuse::downgradeToHttp09(const HTTPRequest& req) {
    HTTPRequest modified = req;
    modified.version = "HTTP/0.9";
    modified.headers.clear();  // HTTP/0.9 doesn't support headers
    return modified;
}

HTTPRequest ProtocolAbuse::downgradeToHttp10(const HTTPRequest& req) {
    HTTPRequest modified = req;
    modified.version = "HTTP/1.0";
    return modified;
}

} // namespace byps
