#ifndef BYPS_TECHNIQUES_PROTOCOL_ABUSE_HPP
#define BYPS_TECHNIQUES_PROTOCOL_ABUSE_HPP

#include "common/types.hpp"
#include <vector>
#include <string>

namespace byps {

class ProtocolAbuse {
public:
    ProtocolAbuse();
    
    // HTTP version manipulation
    std::vector<std::string> getHttpVersions();
    
    // HTTP method variations
    std::vector<std::string> getMethodVariations(const std::string& base_method);
    std::vector<std::string> getCustomMethods();
    
    // Header manipulation
    HTTPRequest addDuplicateHeaders(const HTTPRequest& req);
    HTTPRequest addConflictingHeaders(const HTTPRequest& req);
    
    // Protocol downgrade
    HTTPRequest downgradeToHttp09(const HTTPRequest& req);
    HTTPRequest downgradeToHttp10(const HTTPRequest& req);
    
private:
    std::vector<std::string> http_versions_;
    std::vector<std::string> custom_methods_;
};

} // namespace byps

#endif // BYPS_TECHNIQUES_PROTOCOL_ABUSE_HPP
