#include <gtest/gtest.h>
#include "techniques/path_bypass.hpp"

using namespace byps;

TEST(PathBypassTest, TrailingSlashGeneration) {
    PathBypass pb;
    auto variations = pb.addTrailingSlash("/admin");
    
    EXPECT_FALSE(variations.empty());
    EXPECT_TRUE(std::find(variations.begin(), variations.end(), "/admin/") != variations.end());
}

TEST(PathBypassTest, URLEncodingVariations) {
    PathBypass pb;
    auto variations = pb.urlEncodingVariations("/admin");
    
    EXPECT_FALSE(variations.empty());
    // Should contain encoded variations
    bool has_encoded = false;
    for (const auto& var : variations) {
        if (var.find("%") != std::string::npos) {
            has_encoded = true;
            break;
        }
    }
    EXPECT_TRUE(has_encoded);
}

TEST(PathBypassTest, CaseVariations) {
    PathBypass pb;
    auto variations = pb.caseVariations("/admin");
    
    EXPECT_FALSE(variations.empty());
    EXPECT_TRUE(std::find(variations.begin(), variations.end(), "/ADMIN") != variations.end());
}

TEST(PathBypassTest, GenerateAllVariations) {
    PathBypass pb;
    auto all_variations = pb.generateVariations("/admin");
    
    // Should generate multiple variations
    EXPECT_GT(all_variations.size(), 5);
}
