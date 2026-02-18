#include <gtest/gtest.h>
#include "common/utils.hpp"

using namespace byps::utils;

TEST(UtilsTest, URLEncodingBasic) {
    std::string input = "hello world";
    std::string encoded = urlEncode(input);
    
    EXPECT_NE(encoded, input);
    EXPECT_TRUE(encoded.find("%20") != std::string::npos || encoded.find("+") != std::string::npos);
}

TEST(UtilsTest, URLParsing) {
    std::string url = "http://example.com:8080/path?query=value";
    auto parsed = parseURL(url);
    
    EXPECT_EQ(parsed.scheme, "http");
    EXPECT_EQ(parsed.host, "example.com");
    EXPECT_EQ(parsed.port, 8080);
    EXPECT_EQ(parsed.path, "/path");
    EXPECT_EQ(parsed.query, "query=value");
}

TEST(UtilsTest, StringTrim) {
    std::string input = "  hello  ";
    std::string trimmed = trim(input);
    
    EXPECT_EQ(trimmed, "hello");
}

TEST(UtilsTest, StringSplit) {
    std::string input = "a,b,c,d";
    auto parts = split(input, ',');
    
    EXPECT_EQ(parts.size(), 4);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[3], "d");
}

TEST(UtilsTest, CaseConversion) {
    std::string input = "HeLLo";
    
    EXPECT_EQ(toLower(input), "hello");
    EXPECT_EQ(toUpper(input), "HELLO");
}
