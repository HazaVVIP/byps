#ifndef BYPS_TECHNIQUES_PATH_BYPASS_HPP
#define BYPS_TECHNIQUES_PATH_BYPASS_HPP

#include "common/types.hpp"
#include <vector>
#include <string>

namespace byps {

class PathBypass {
public:
    PathBypass();
    
    // Generate path bypass variations
    std::vector<std::string> generateVariations(const std::string& path);
    
    // Specific techniques
    std::vector<std::string> addTrailingSlash(const std::string& path);
    std::vector<std::string> urlEncodingVariations(const std::string& path);
    std::vector<std::string> caseVariations(const std::string& path);
    std::vector<std::string> pathTraversal(const std::string& path);
    std::vector<std::string> nullByteInjection(const std::string& path);
    std::vector<std::string> dotSegments(const std::string& path);
    std::vector<std::string> semicolonBypass(const std::string& path);
    std::vector<std::string> percentEncodedDots(const std::string& path);
    
private:
    bool isValidPath(const std::string& path);
};

} // namespace byps

#endif // BYPS_TECHNIQUES_PATH_BYPASS_HPP
