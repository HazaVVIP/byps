#ifndef BYPS_TECHNIQUES_HEADER_FORGE_HPP
#define BYPS_TECHNIQUES_HEADER_FORGE_HPP

#include "common/types.hpp"
#include <vector>
#include <string>
#include <map>

namespace byps {

class HeaderForge {
public:
    HeaderForge();
    
    // Generate header bypass variations
    std::map<std::string, std::string> generateForwardedForHeaders();
    std::map<std::string, std::string> generateRewriteHeaders(const std::string& target_path);
    std::map<std::string, std::string> generateHostHeaders();
    std::map<std::string, std::string> generateCustomAuthHeaders();
    std::map<std::string, std::string> generateBypassHeaders();
    
    // Specific techniques
    std::vector<std::string> getCommonIPSpoofValues();
    std::vector<std::string> getCommonHostValues();
    
private:
    std::vector<std::string> internal_ips_;
    std::vector<std::string> localhost_variants_;
};

} // namespace byps

#endif // BYPS_TECHNIQUES_HEADER_FORGE_HPP
