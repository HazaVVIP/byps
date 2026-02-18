#include "techniques/unicode.hpp"
#include <map>
#include <sstream>
#include <cstring>
#include <cstdint>

namespace byps {

Unicode::Unicode() {}

// Helper function to decode UTF-8 character
static std::pair<uint32_t, size_t> decodeUtf8(const std::string& str, size_t pos) {
    if (pos >= str.size()) return {0, 0};
    
    unsigned char c = str[pos];
    
    if ((c & 0x80) == 0) {
        return {c, 1};
    } else if ((c & 0xE0) == 0xC0) {
        if (pos + 1 >= str.size()) return {0, 0};
        if ((str[pos + 1] & 0xC0) != 0x80) return {0, 1};
        uint32_t codepoint = ((c & 0x1F) << 6) | (str[pos + 1] & 0x3F);
        if (codepoint < 0x80) return {0, 1};
        return {codepoint, 2};
    } else if ((c & 0xF0) == 0xE0) {
        if (pos + 2 >= str.size()) return {0, 0};
        if ((str[pos + 1] & 0xC0) != 0x80 || (str[pos + 2] & 0xC0) != 0x80) return {0, 1};
        uint32_t codepoint = ((c & 0x0F) << 12) | 
                             ((str[pos + 1] & 0x3F) << 6) | 
                             (str[pos + 2] & 0x3F);
        if (codepoint < 0x800) return {0, 1};
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return {0, 1};
        return {codepoint, 3};
    } else if ((c & 0xF8) == 0xF0) {
        if (pos + 3 >= str.size()) return {0, 0};
        if ((str[pos + 1] & 0xC0) != 0x80 || (str[pos + 2] & 0xC0) != 0x80 || (str[pos + 3] & 0xC0) != 0x80) return {0, 1};
        uint32_t codepoint = ((c & 0x07) << 18) | 
                             ((str[pos + 1] & 0x3F) << 12) | 
                             ((str[pos + 2] & 0x3F) << 6) | 
                             (str[pos + 3] & 0x3F);
        if (codepoint < 0x10000 || codepoint > 0x10FFFF) return {0, 1};
        return {codepoint, 4};
    }
    
    return {0, 1};
}

// Helper function to encode UTF-8 character
static std::string encodeUtf8(uint32_t codepoint) {
    std::string result;
    
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return result;
    if (codepoint > 0x10FFFF) return result;
    
    if (codepoint < 0x80) {
        result += static_cast<char>(codepoint);
    } else if (codepoint < 0x800) {
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint < 0x10000) {
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint < 0x110000) {
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    
    return result;
}

std::vector<std::string> Unicode::generateNormalizationVariations(const std::string& str) {
    std::vector<std::string> variations;
    variations.push_back(str);
    
    // NFD-like decomposition: Convert characters to decomposed forms
    // Example: é (U+00E9) -> e (U+0065) + ́ (U+0301)
    static const std::map<uint32_t, std::string> decompositions = {
        {0x00C0, "\x41\xCC\x80"}, // À -> A + grave
        {0x00C1, "\x41\xCC\x81"}, // Á -> A + acute
        {0x00C2, "\x41\xCC\x82"}, // Â -> A + circumflex
        {0x00C3, "\x41\xCC\x83"}, // Ã -> A + tilde
        {0x00C4, "\x41\xCC\x88"}, // Ä -> A + diaeresis
        {0x00C8, "\x45\xCC\x80"}, // È -> E + grave
        {0x00C9, "\x45\xCC\x81"}, // É -> E + acute
        {0x00CA, "\x45\xCC\x82"}, // Ê -> E + circumflex
        {0x00CB, "\x45\xCC\x88"}, // Ë -> E + diaeresis
        {0x00CC, "\x49\xCC\x80"}, // Ì -> I + grave
        {0x00CD, "\x49\xCC\x81"}, // Í -> I + acute
        {0x00CE, "\x49\xCC\x82"}, // Î -> I + circumflex
        {0x00CF, "\x49\xCC\x88"}, // Ï -> I + diaeresis
        {0x00D1, "\x4E\xCC\x83"}, // Ñ -> N + tilde
        {0x00D2, "\x4F\xCC\x80"}, // Ò -> O + grave
        {0x00D3, "\x4F\xCC\x81"}, // Ó -> O + acute
        {0x00D4, "\x4F\xCC\x82"}, // Ô -> O + circumflex
        {0x00D5, "\x4F\xCC\x83"}, // Õ -> O + tilde
        {0x00D6, "\x4F\xCC\x88"}, // Ö -> O + diaeresis
        {0x00D9, "\x55\xCC\x80"}, // Ù -> U + grave
        {0x00DA, "\x55\xCC\x81"}, // Ú -> U + acute
        {0x00DB, "\x55\xCC\x82"}, // Û -> U + circumflex
        {0x00DC, "\x55\xCC\x88"}, // Ü -> U + diaeresis
        {0x00DD, "\x59\xCC\x81"}, // Ý -> Y + acute
        {0x00E0, "\x61\xCC\x80"}, // à -> a + grave
        {0x00E1, "\x61\xCC\x81"}, // á -> a + acute
        {0x00E2, "\x61\xCC\x82"}, // â -> a + circumflex
        {0x00E3, "\x61\xCC\x83"}, // ã -> a + tilde
        {0x00E4, "\x61\xCC\x88"}, // ä -> a + diaeresis
        {0x00E8, "\x65\xCC\x80"}, // è -> e + grave
        {0x00E9, "\x65\xCC\x81"}, // é -> e + acute
        {0x00EA, "\x65\xCC\x82"}, // ê -> e + circumflex
        {0x00EB, "\x65\xCC\x88"}, // ë -> e + diaeresis
        {0x00EC, "\x69\xCC\x80"}, // ì -> i + grave
        {0x00ED, "\x69\xCC\x81"}, // í -> i + acute
        {0x00EE, "\x69\xCC\x82"}, // î -> i + circumflex
        {0x00EF, "\x69\xCC\x88"}, // ï -> i + diaeresis
        {0x00F1, "\x6E\xCC\x83"}, // ñ -> n + tilde
        {0x00F2, "\x6F\xCC\x80"}, // ò -> o + grave
        {0x00F3, "\x6F\xCC\x81"}, // ó -> o + acute
        {0x00F4, "\x6F\xCC\x82"}, // ô -> o + circumflex
        {0x00F5, "\x6F\xCC\x83"}, // õ -> o + tilde
        {0x00F6, "\x6F\xCC\x88"}, // ö -> o + diaeresis
        {0x00F9, "\x75\xCC\x80"}, // ù -> u + grave
        {0x00FA, "\x75\xCC\x81"}, // ú -> u + acute
        {0x00FB, "\x75\xCC\x82"}, // û -> u + circumflex
        {0x00FC, "\x75\xCC\x88"}, // ü -> u + diaeresis
        {0x00FD, "\x79\xCC\x81"}, // ý -> y + acute
        {0x00FF, "\x79\xCC\x88"}, // ÿ -> y + diaeresis
    };
    
    // Generate NFD-like variation
    std::string nfd;
    for (size_t i = 0; i < str.size();) {
        auto [codepoint, len] = decodeUtf8(str, i);
        if (len == 0) break;
        
        auto it = decompositions.find(codepoint);
        if (it != decompositions.end()) {
            nfd += it->second;
        } else {
            nfd += str.substr(i, len);
        }
        i += len;
    }
    if (nfd != str) {
        variations.push_back(nfd);
    }
    
    // Add zero-width characters between letters (NFD-like with zero-width)
    std::string zwj_variation;
    for (size_t i = 0; i < str.size();) {
        auto [codepoint, len] = decodeUtf8(str, i);
        if (len == 0) break;
        
        zwj_variation += str.substr(i, len);
        
        // Add zero-width joiner after letters
        if ((codepoint >= 'a' && codepoint <= 'z') || 
            (codepoint >= 'A' && codepoint <= 'Z')) {
            zwj_variation += "\xE2\x80\x8D"; // U+200D Zero Width Joiner
        }
        i += len;
    }
    if (zwj_variation != str) {
        variations.push_back(zwj_variation);
    }
    
    // Add zero-width space variation
    std::string zws_variation;
    for (size_t i = 0; i < str.size();) {
        auto [codepoint, len] = decodeUtf8(str, i);
        if (len == 0) break;
        
        zws_variation += str.substr(i, len);
        
        // Add zero-width space after letters
        if ((codepoint >= 'a' && codepoint <= 'z') || 
            (codepoint >= 'A' && codepoint <= 'Z')) {
            zws_variation += "\xE2\x80\x8B"; // U+200B Zero Width Space
        }
        i += len;
    }
    if (zws_variation != str) {
        variations.push_back(zws_variation);
    }
    
    // Add zero-width non-joiner variation
    std::string zwnj_variation;
    for (size_t i = 0; i < str.size();) {
        auto [codepoint, len] = decodeUtf8(str, i);
        if (len == 0) break;
        
        zwnj_variation += str.substr(i, len);
        
        // Add zero-width non-joiner after letters
        if ((codepoint >= 'a' && codepoint <= 'z') || 
            (codepoint >= 'A' && codepoint <= 'Z')) {
            zwnj_variation += "\xE2\x80\x8C"; // U+200C Zero Width Non-Joiner
        }
        i += len;
    }
    if (zwnj_variation != str) {
        variations.push_back(zwnj_variation);
    }
    
    // Add variation with bidirectional override markers
    // Right-to-Left Override (U+202E) and Pop Directional Formatting (U+202C)
    std::string bidi_variation = "\xE2\x80\xAE" + str + "\xE2\x80\xAC";
    variations.push_back(bidi_variation);
    
    // Left-to-Right Override
    std::string ltr_variation = "\xE2\x80\xAD" + str + "\xE2\x80\xAC";
    variations.push_back(ltr_variation);
    
    return variations;
}

std::vector<std::string> Unicode::generateHomographs(const std::string& str) {
    std::vector<std::string> variations;
    variations.push_back(str);
    
    // Homograph map: ASCII character -> visually similar Unicode characters
    static const std::map<char, std::vector<std::string>> homographs = {
        // Latin letters to Cyrillic and other confusables
        {'a', {"\xD0\xB0", "\xC3\xA0", "\xC3\xA1", "\xC3\xA2", "\xE1\xB4\x80"}}, // а(Cyrillic), à, á, â, ᴀ
        {'A', {"\xD0\x90", "\xC3\x80", "\xC3\x81", "\xC3\x82", "\xCE\x91"}}, // А(Cyrillic), À, Á, Â, Α(Greek)
        {'c', {"\xD1\x81", "\xC3\xA7", "\xE1\xB4\x84"}}, // с(Cyrillic), ç, ᴄ
        {'C', {"\xD0\xA1", "\xC3\x87", "\xCE\x9C"}}, // С(Cyrillic), Ç, Μ(Greek)
        {'e', {"\xD0\xB5", "\xC3\xA8", "\xC3\xA9", "\xC3\xAA", "\xC9\x9B"}}, // е(Cyrillic), è, é, ê, ɛ
        {'E', {"\xD0\x95", "\xC3\x88", "\xC3\x89", "\xC3\x8A", "\xCE\x95"}}, // Е(Cyrillic), È, É, Ê, Ε(Greek)
        {'i', {"\xD1\x96", "\xC3\xAC", "\xC3\xAD", "\xC4\xB1", "\xE2\x80\xA4"}}, // і(Cyrillic), ì, í, ı, ⁱ
        {'I', {"\xD0\x86", "\xC3\x8C", "\xC3\x8D", "\xCE\x99", "\xE2\x85\xA0"}}, // І(Cyrillic), Ì, Í, Ι(Greek), Ⅰ
        {'j', {"\xD1\x98", "\xC4\xB5"}}, // ј(Cyrillic), ĵ
        {'J', {"\xD0\x88", "\xC4\xB4"}}, // Ј(Cyrillic), Ĵ
        {'o', {"\xD0\xBE", "\xC3\xB2", "\xC3\xB3", "\xC3\xB4", "\xCE\xBF", "\x30"}}, // о(Cyrillic), ò, ó, ô, ο(Greek), 0
        {'O', {"\xD0\x9E", "\xC3\x92", "\xC3\x93", "\xC3\x94", "\xCE\x9F", "\x30"}}, // О(Cyrillic), Ò, Ó, Ô, Ο(Greek), 0
        {'p', {"\xD1\x80", "\xC3\xBE"}}, // р(Cyrillic), þ
        {'P', {"\xD0\xA0", "\xCE\xA1"}}, // Р(Cyrillic), Ρ(Greek)
        {'s', {"\xD1\x95", "\xC5\x9F", "\xC5\xA1"}}, // ѕ(Cyrillic), ş, š
        {'S', {"\xD0\x85", "\xC5\x9E", "\xC5\xA0"}}, // Ѕ(Cyrillic), Ş, Š
        {'x', {"\xD1\x85", "\xC3\x97"}}, // х(Cyrillic), ×
        {'X', {"\xD0\xA5", "\xCE\xA7"}}, // Х(Cyrillic), Χ(Greek)
        {'y', {"\xD1\x83", "\xC3\xBD", "\xC3\xBF"}}, // у(Cyrillic), ý, ÿ
        {'Y', {"\xD0\xA3", "\xC3\x9D", "\xCE\xA5"}}, // У(Cyrillic), Ý, Υ(Greek)
        {'B', {"\xD0\x92", "\xCE\x92"}}, // В(Cyrillic), Β(Greek)
        {'H', {"\xD0\x9D", "\xCE\x97"}}, // Н(Cyrillic), Η(Greek)
        {'K', {"\xD0\x9A", "\xCE\x9A"}}, // К(Cyrillic), Κ(Greek)
        {'M', {"\xD0\x9C", "\xCE\x9C"}}, // М(Cyrillic), Μ(Greek)
        {'N', {"\xD0\x9D", "\xC3\x91"}}, // Н(Cyrillic), Ñ
        {'T', {"\xD0\xA2", "\xCE\xA4"}}, // Т(Cyrillic), Τ(Greek)
        {'Z', {"\xD0\x96", "\xC5\xB9", "\xC5\xBD"}}, // Ж(Cyrillic), Ź, Ž
        
        // Numbers to visually similar characters
        {'0', {"\xD0\x9E", "\xCE\x9F", "\x4F"}}, // О(Cyrillic), Ο(Greek), O
        {'1', {"\xD0\x86", "\xE2\x85\xA0", "\x6C", "\x49"}}, // І(Cyrillic), Ⅰ, l, I
        {'3', {"\xD0\x97", "\xC6\x90"}}, // З(Cyrillic), Ɛ
        {'5', {"\xD0\x85", "\xE1\xB4\xA8"}}, // Ѕ(Cyrillic), ᵨ
        {'6', {"\xD0\xB1", "\xC6\x93"}}, // б(Cyrillic), ƍ
        {'8', {"\xD0\x92", "\xC9\xB9"}}, // В(Cyrillic), ɹ
    };
    
    // Generate variations by substituting each character
    for (const auto& [ascii_char, replacements] : homographs) {
        for (const auto& replacement : replacements) {
            std::string variation;
            bool changed = false;
            
            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == ascii_char) {
                    variation += replacement;
                    changed = true;
                } else {
                    variation += str[i];
                }
            }
            
            if (changed && variation != str) {
                variations.push_back(variation);
            }
        }
    }
    
    // Generate mixed variations (multiple substitutions)
    std::string mixed;
    bool has_substitution = false;
    
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char uc = static_cast<unsigned char>(str[i]);
        auto it = homographs.find(static_cast<char>(uc));
        if (it != homographs.end() && !it->second.empty()) {
            mixed += it->second[0];
            has_substitution = true;
        } else {
            mixed += str[i];
        }
    }
    
    if (has_substitution && mixed != str) {
        variations.push_back(mixed);
    }
    
    // Add full-width character variations for ASCII
    std::string fullwidth;
    for (unsigned char uc : str) {
        if (uc >= 0x21 && uc <= 0x7E) {
            // Convert ASCII to full-width form (U+FF01 to U+FF5E)
            uint32_t fw = 0xFF00 + uc;
            fullwidth += encodeUtf8(fw);
        } else {
            fullwidth += static_cast<char>(uc);
        }
    }
    if (fullwidth != str) {
        variations.push_back(fullwidth);
    }
    
    // Mathematical Alphanumeric Symbols (Bold)
    std::string math_bold;
    for (unsigned char uc : str) {
        if (uc >= 'A' && uc <= 'Z') {
            uint32_t bold = 0x1D400 + (uc - 'A');
            math_bold += encodeUtf8(bold);
        } else if (uc >= 'a' && uc <= 'z') {
            uint32_t bold = 0x1D41A + (uc - 'a');
            math_bold += encodeUtf8(bold);
        } else if (uc >= '0' && uc <= '9') {
            uint32_t bold = 0x1D7CE + (uc - '0');
            math_bold += encodeUtf8(bold);
        } else {
            math_bold += static_cast<char>(uc);
        }
    }
    if (math_bold != str) {
        variations.push_back(math_bold);
    }
    
    return variations;
}

} // namespace byps
