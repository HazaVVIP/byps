#include "common/utils.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <cctype>

namespace byps {
namespace utils {

std::string trim(const std::string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        start++;
    }
    
    auto end = str.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    
    return std::string(start, end + 1);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
    if (parts.empty()) {
        return "";
    }
    
    std::stringstream ss;
    ss << parts[0];
    for (size_t i = 1; i < parts.size(); i++) {
        ss << delimiter << parts[i];
    }
    return ss.str();
}

std::string urlEncode(const std::string& str) {
    static const char hex_chars[] = "0123456789ABCDEF";
    std::string encoded;
    
    for (unsigned char c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            encoded += '%';
            encoded += hex_chars[(c >> 4) & 0x0F];
            encoded += hex_chars[c & 0x0F];
        }
    }
    
    return encoded;
}

std::string urlDecode(const std::string& str) {
    std::string decoded;
    
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::stringstream ss;
            ss << std::hex << str.substr(i + 1, 2);
            ss >> value;
            decoded += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    
    return decoded;
}

std::string urlEncodeMultiple(const std::string& str, int levels) {
    std::string result = str;
    for (int i = 0; i < levels; i++) {
        result = urlEncode(result);
    }
    return result;
}

std::string base64Encode(const std::vector<uint8_t>& data) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string encoded;
    int i = 0;
    int j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];
    
    for (uint8_t byte : data) {
        char_array_3[i++] = byte;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                encoded += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++) {
            encoded += base64_chars[char_array_4[j]];
        }
        
        while (i++ < 3) {
            encoded += '=';
        }
    }
    
    return encoded;
}

std::vector<uint8_t> base64Decode(const std::string& str) {
    // Simplified base64 decoder
    std::vector<uint8_t> decoded;
    // Implementation omitted for brevity
    return decoded;
}

std::string hexEncode(const std::vector<uint8_t>& data) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string encoded;
    
    for (uint8_t byte : data) {
        encoded += hex_chars[(byte >> 4) & 0x0F];
        encoded += hex_chars[byte & 0x0F];
    }
    
    return encoded;
}

std::vector<uint8_t> hexDecode(const std::string& str) {
    std::vector<uint8_t> decoded;
    
    for (size_t i = 0; i < str.length(); i += 2) {
        if (i + 1 < str.length()) {
            int value;
            std::stringstream ss;
            ss << std::hex << str.substr(i, 2);
            ss >> value;
            decoded.push_back(static_cast<uint8_t>(value));
        }
    }
    
    return decoded;
}

ParsedURL parseURL(const std::string& url) {
    ParsedURL parsed;
    
    // Find scheme
    size_t scheme_end = url.find("://");
    if (scheme_end != std::string::npos) {
        parsed.scheme = url.substr(0, scheme_end);
        scheme_end += 3;
    } else {
        scheme_end = 0;
        parsed.scheme = "http";
    }
    
    // Find host and port
    size_t path_start = url.find('/', scheme_end);
    std::string host_port = url.substr(scheme_end, 
        path_start == std::string::npos ? std::string::npos : path_start - scheme_end);
    
    size_t port_start = host_port.find(':');
    if (port_start != std::string::npos) {
        parsed.host = host_port.substr(0, port_start);
        parsed.port = std::stoi(host_port.substr(port_start + 1));
    } else {
        parsed.host = host_port;
        parsed.port = (parsed.scheme == "https") ? 443 : 80;
    }
    
    // Find path
    if (path_start != std::string::npos) {
        size_t query_start = url.find('?', path_start);
        size_t fragment_start = url.find('#', path_start);
        
        if (query_start != std::string::npos) {
            parsed.path = url.substr(path_start, query_start - path_start);
            if (fragment_start != std::string::npos) {
                parsed.query = url.substr(query_start + 1, fragment_start - query_start - 1);
                parsed.fragment = url.substr(fragment_start + 1);
            } else {
                parsed.query = url.substr(query_start + 1);
            }
        } else if (fragment_start != std::string::npos) {
            parsed.path = url.substr(path_start, fragment_start - path_start);
            parsed.fragment = url.substr(fragment_start + 1);
        } else {
            parsed.path = url.substr(path_start);
        }
    } else {
        parsed.path = "/";
    }
    
    return parsed;
}

std::string buildURL(const ParsedURL& parsed) {
    std::stringstream ss;
    ss << parsed.scheme << "://" << parsed.host;
    
    if ((parsed.scheme == "http" && parsed.port != 80) ||
        (parsed.scheme == "https" && parsed.port != 443)) {
        ss << ":" << parsed.port;
    }
    
    ss << parsed.path;
    
    if (!parsed.query.empty()) {
        ss << "?" << parsed.query;
    }
    
    if (!parsed.fragment.empty()) {
        ss << "#" << parsed.fragment;
    }
    
    return ss.str();
}

std::string generateRandomString(size_t length) {
    static const char charset[] = 
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; i++) {
        result += charset[dis(gen)];
    }
    
    return result;
}

std::vector<uint8_t> generateRandomBytes(size_t length) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 255);
    
    std::vector<uint8_t> result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; i++) {
        result.push_back(static_cast<uint8_t>(dis(gen)));
    }
    
    return result;
}

long long getCurrentTimeMs() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

long long getCurrentTimeMicros() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

std::vector<std::string> generateCaseVariations(const std::string& str) {
    std::vector<std::string> variations;
    
    // All uppercase
    variations.push_back(toUpper(str));
    
    // All lowercase
    variations.push_back(toLower(str));
    
    // First char upper, rest lower
    if (!str.empty()) {
        std::string capitalized = toLower(str);
        capitalized[0] = std::toupper(capitalized[0]);
        variations.push_back(capitalized);
    }
    
    return variations;
}

} // namespace utils
} // namespace byps
