#ifndef BYPS_COMMON_UTILS_HPP
#define BYPS_COMMON_UTILS_HPP

#include <string>
#include <vector>
#include <map>

namespace byps {
namespace utils {

// String manipulation
std::string trim(const std::string& str);
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

// URL encoding/decoding
std::string urlEncode(const std::string& str);
std::string urlDecode(const std::string& str);
std::string urlEncodeMultiple(const std::string& str, int levels);

// Base64 encoding/decoding
std::string base64Encode(const std::vector<uint8_t>& data);
std::vector<uint8_t> base64Decode(const std::string& str);

// Hex encoding/decoding
std::string hexEncode(const std::vector<uint8_t>& data);
std::vector<uint8_t> hexDecode(const std::string& str);

// URL parsing
struct ParsedURL {
    std::string scheme;
    std::string host;
    uint16_t port;
    std::string path;
    std::string query;
    std::string fragment;
    
    ParsedURL() : port(0) {}
};

ParsedURL parseURL(const std::string& url);
std::string buildURL(const ParsedURL& parsed);

// Random generation
std::string generateRandomString(size_t length);
std::vector<uint8_t> generateRandomBytes(size_t length);

// Timing utilities
long long getCurrentTimeMs();
long long getCurrentTimeMicros();

// Case variations
std::vector<std::string> generateCaseVariations(const std::string& str);

} // namespace utils
} // namespace byps

#endif // BYPS_COMMON_UTILS_HPP
