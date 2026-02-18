#include "techniques/encoding.hpp"
#include "common/utils.hpp"
#include <sstream>
#include <iomanip>
#include <random>

namespace byps {

Encoding::Encoding() {
}

std::string Encoding::singleUrlEncode(const std::string& str) {
    return utils::urlEncode(str);
}

std::string Encoding::doubleUrlEncode(const std::string& str) {
    return utils::urlEncodeMultiple(str, 2);
}

std::string Encoding::tripleUrlEncode(const std::string& str) {
    return utils::urlEncodeMultiple(str, 3);
}

std::string Encoding::htmlEntityEncode(const std::string& str) {
    std::stringstream ss;
    for (char c : str) {
        ss << "&#" << static_cast<int>(static_cast<unsigned char>(c)) << ";";
    }
    return ss.str();
}

std::string Encoding::unicodeEscape(const std::string& str) {
    std::stringstream ss;
    for (char c : str) {
        ss << "\\u00" << std::hex << std::setfill('0') << std::setw(2) 
           << static_cast<int>(static_cast<unsigned char>(c));
    }
    return ss.str();
}

std::string Encoding::mixedEncode(const std::string& str) {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);
    
    for (char c : str) {
        int encoding_type = dis(gen);
        if (encoding_type == 0) {
            ss << c;  // No encoding
        } else if (encoding_type == 1) {
            // URL encode
            ss << "%" << std::hex << std::setfill('0') << std::setw(2)
               << static_cast<int>(static_cast<unsigned char>(c));
        } else {
            // HTML entity
            ss << "&#" << static_cast<int>(static_cast<unsigned char>(c)) << ";";
        }
    }
    return ss.str();
}

std::string Encoding::partialUrlEncode(const std::string& str, double percentage) {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (char c : str) {
        if (dis(gen) < percentage) {
            ss << "%" << std::hex << std::setfill('0') << std::setw(2)
               << static_cast<int>(static_cast<unsigned char>(c));
        } else {
            ss << c;
        }
    }
    return ss.str();
}

std::vector<std::string> Encoding::generateEncodingVariations(const std::string& str) {
    std::vector<std::string> variations;
    
    variations.push_back(singleUrlEncode(str));
    variations.push_back(doubleUrlEncode(str));
    variations.push_back(tripleUrlEncode(str));
    variations.push_back(htmlEntityEncode(str));
    variations.push_back(unicodeEscape(str));
    variations.push_back(mixedEncode(str));
    variations.push_back(partialUrlEncode(str, 0.5));
    
    return variations;
}

bool Encoding::shouldEncode(char c) {
    return !(std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~');
}

} // namespace byps
