#ifndef BYPS_TECHNIQUES_ENCODING_HPP
#define BYPS_TECHNIQUES_ENCODING_HPP

#include <string>
#include <vector>

namespace byps {

class Encoding {
public:
    Encoding();
    
    // URL encoding variations
    std::string singleUrlEncode(const std::string& str);
    std::string doubleUrlEncode(const std::string& str);
    std::string tripleUrlEncode(const std::string& str);
    
    // Character encoding
    std::string htmlEntityEncode(const std::string& str);
    std::string unicodeEscape(const std::string& str);
    
    // Mixed encoding
    std::string mixedEncode(const std::string& str);
    
    // Partial encoding (encode only some characters)
    std::string partialUrlEncode(const std::string& str, double percentage);
    
    // Generate all encoding variations
    std::vector<std::string> generateEncodingVariations(const std::string& str);
    
private:
    bool shouldEncode(char c);
};

} // namespace byps

#endif // BYPS_TECHNIQUES_ENCODING_HPP
