#include "byps.h"
#include "common/types.hpp"
#include "common/logger.hpp"
#include "common/error.hpp"
#include "common/utils.hpp"
#include "techniques/path_bypass.hpp"
#include "techniques/header_forge.hpp"
#include "techniques/encoding.hpp"
#include "network/http_client.hpp"
#include <cstring>
#include <memory>
#include <sstream>
#include <iomanip>

using namespace byps;

// Internal structure to hold C++ objects
struct BypsEngine {
    std::unique_ptr<PathBypass> path_bypass;
    std::unique_ptr<HeaderForge> header_forge;
    std::unique_ptr<Encoding> encoding;
    std::unique_ptr<network::HttpClient> http_client;
    std::string last_error;
    
    BypsEngine() 
        : path_bypass(std::make_unique<PathBypass>()),
          header_forge(std::make_unique<HeaderForge>()),
          encoding(std::make_unique<Encoding>()),
          http_client(std::make_unique<network::HttpClient>()) {
        // Configure HTTP client for bypass testing
        http_client->setTimeout(30000);
        http_client->setFollowRedirects(false);
        http_client->setVerifySSL(false);
    }
};

// Helper function to convert std::string to C string
static char* string_to_c_str(const std::string& str) {
    char* result = static_cast<char*>(malloc(str.length() + 1));
    if (result) {
        std::strcpy(result, str.c_str());
    }
    return result;
}

extern "C" {

BypsEngine* byps_engine_new(void) {
    try {
        return new BypsEngine();
    } catch (const std::exception& e) {
        Logger::getInstance().error(std::string("Failed to create engine: ") + e.what());
        return nullptr;
    }
}

void byps_engine_free(BypsEngine* engine) {
    delete engine;
}

int byps_engine_scan(BypsEngine* engine,
                     const char* url,
                     const char* config_json,
                     char** result_json) {
    if (!engine || !url || !result_json) {
        return BYPS_ERROR_INVALID_URL;
    }
    
    try {
        Logger::getInstance().info(std::string("Starting scan of: ") + url);
        
        // Parse URL
        auto parsed = utils::parseURL(url);
        
        // Generate bypass variations
        auto path_variations = engine->path_bypass->generateVariations(parsed.path);
        
        // Build result JSON (simplified for now)
        std::stringstream ss;
        ss << "{\"url\":\"" << url << "\",";
        ss << "\"variations\":[";
        for (size_t i = 0; i < path_variations.size() && i < 10; i++) {  // Limit to 10 for testing
            // Escape special characters in JSON
            std::string escaped = path_variations[i];
            // Simple escaping for now
            size_t pos = 0;
            while ((pos = escaped.find('\\', pos)) != std::string::npos) {
                escaped.replace(pos, 1, "\\\\");
                pos += 2;
            }
            pos = 0;
            while ((pos = escaped.find('"', pos)) != std::string::npos) {
                escaped.insert(pos, "\\");
                pos += 2;
            }
            
            ss << "\"" << escaped << "\"";
            if (i < path_variations.size() - 1 && i < 9) {
                ss << ",";
            }
        }
        ss << "]}";
        
        *result_json = string_to_c_str(ss.str());
        return BYPS_SUCCESS;
        
    } catch (const BypsException& e) {
        engine->last_error = e.what();
        Logger::getInstance().error(e.what());
        return static_cast<int>(e.code());
    } catch (const std::exception& e) {
        engine->last_error = e.what();
        Logger::getInstance().error(e.what());
        return BYPS_ERROR_UNKNOWN;
    }
}

int byps_engine_test_technique(BypsEngine* engine,
                               const char* url,
                               const char* technique_name,
                               char** result_json) {
    if (!engine || !url || !technique_name || !result_json) {
        return BYPS_ERROR_INVALID_URL;
    }
    
    try {
        Logger::getInstance().info(std::string("Testing technique: ") + technique_name + " on " + url);
        
        auto parsed = utils::parseURL(url);
        std::vector<std::string> variations;
        
        std::string technique(technique_name);
        if (technique == "path_bypass") {
            variations = engine->path_bypass->generateVariations(parsed.path);
        } else if (technique == "url_encoding") {
            variations = engine->encoding->generateEncodingVariations(parsed.path);
        }
        
        // Build result JSON
        std::stringstream ss;
        ss << "{\"technique\":\"" << technique_name << "\",";
        ss << "\"url\":\"" << url << "\",";
        ss << "\"variations\":[";
        for (size_t i = 0; i < variations.size() && i < 10; i++) {
            // Escape special characters
            std::string escaped = variations[i];
            size_t pos = 0;
            while ((pos = escaped.find('\\', pos)) != std::string::npos) {
                escaped.replace(pos, 1, "\\\\");
                pos += 2;
            }
            pos = 0;
            while ((pos = escaped.find('"', pos)) != std::string::npos) {
                escaped.insert(pos, "\\");
                pos += 2;
            }
            
            ss << "\"" << escaped << "\"";
            if (i < variations.size() - 1 && i < 9) {
                ss << ",";
            }
        }
        ss << "]}";
        
        *result_json = string_to_c_str(ss.str());
        return BYPS_SUCCESS;
        
    } catch (const std::exception& e) {
        engine->last_error = e.what();
        return BYPS_ERROR_UNKNOWN;
    }
}

int byps_engine_detect_waf(BypsEngine* engine,
                           const char* url,
                           char** waf_info_json) {
    if (!engine || !url || !waf_info_json) {
        return BYPS_ERROR_INVALID_URL;
    }
    
    try {
        // Placeholder WAF detection
        std::string json = "{\"type\":\"unknown\",\"confidence\":0.0}";
        *waf_info_json = string_to_c_str(json);
        return BYPS_SUCCESS;
    } catch (const std::exception& e) {
        engine->last_error = e.what();
        return BYPS_ERROR_UNKNOWN;
    }
}

int byps_engine_test_variations(BypsEngine* engine,
                               const char* base_url,
                               const char* config_json,
                               char** result_json) {
    if (!engine || !base_url || !result_json) {
        return BYPS_ERROR_INVALID_URL;
    }
    
    try {
        Logger::getInstance().info(std::string("Testing variations for: ") + base_url);
        
        // Parse URL
        auto parsed = utils::parseURL(base_url);
        
        // Generate bypass variations
        auto path_variations = engine->path_bypass->generateVariations(parsed.path);
        
        // Test baseline request first
        std::string baseline_url = base_url;
        auto baseline_response = engine->http_client->get(baseline_url);
        int baseline_status = baseline_response.status_code;
        size_t baseline_size = baseline_response.body.length();
        
        Logger::getInstance().info("Baseline status: " + std::to_string(baseline_status) + 
                                 ", size: " + std::to_string(baseline_size));
        
        // Build result JSON with tested variations
        std::stringstream ss;
        ss << "{\"baseline\":{";
        ss << "\"status\":" << baseline_status << ",";
        ss << "\"size\":" << baseline_size << ",";
        ss << "\"time\":" << baseline_response.response_time_ms;
        ss << "},\"variations\":[";
        
        bool first = true;
        int successful_bypasses = 0;
        int total_tested = 0;
        
        for (const auto& variation : path_variations) {
            if (total_tested >= 50) break; // Limit for safety
            
            // Construct full URL with variation
            std::string test_url = parsed.scheme + "://" + parsed.host;
            if (parsed.port != 80 && parsed.port != 443) {
                test_url += ":" + std::to_string(parsed.port);
            }
            test_url += variation;
            if (!parsed.query.empty()) {
                test_url += "?" + parsed.query;
            }
            
            // Execute HTTP request
            auto response = engine->http_client->get(test_url);
            total_tested++;
            
            // Check if bypass was successful
            bool bypass_success = false;
            std::string bypass_reason = "failed";
            
            // Skip if request failed (status 0)
            if (response.status_code == 0) {
                bypass_success = false;
                bypass_reason = "connection_failed";
            }
            // Success criteria: status code changed from 403/401 to 200/30x
            else if (baseline_status >= 400 && response.status_code >= 200 && response.status_code < 400) {
                // Additional check: verify it's not a soft 404 or redirect to error page
                // Soft 404s often have small bodies or specific patterns
                if (response.body.length() < 100) {
                    // Likely a minimal response, might be soft 404
                    bypass_success = false;
                    bypass_reason = "possible_soft_404";
                } else if (response.body.find("404") != std::string::npos || 
                          response.body.find("not found") != std::string::npos ||
                          response.body.find("Not Found") != std::string::npos) {
                    // Body contains "404" or "not found" text
                    bypass_success = false;
                    bypass_reason = "soft_404_detected";
                } else {
                    // Status changed and body looks legitimate
                    bypass_success = true;
                    bypass_reason = "status_change";
                    successful_bypasses++;
                }
            } else if (response.status_code == baseline_status) {
                // Check for significant size difference (might indicate different content)
                if (baseline_size > 0 && response.body.length() > 0) {
                    double size_diff_ratio = std::abs(static_cast<double>(response.body.length()) - 
                                                      static_cast<double>(baseline_size)) / baseline_size;
                    // Only flag if size difference is significant (>30%) AND response is larger
                    if (size_diff_ratio > 0.3 && response.body.length() > baseline_size) {
                        bypass_success = true;
                        bypass_reason = "size_difference";
                        successful_bypasses++;
                    }
                }
            }
            
            // Add to JSON
            if (!first) ss << ",";
            first = false;
            
            ss << "{";
            ss << "\"variation\":\"";
            // Escape the variation string for JSON
            for (size_t i = 0; i < variation.length(); ++i) {
                unsigned char c = static_cast<unsigned char>(variation[i]);
                if (c == '\0') ss << "\\u0000"; // Null byte
                else if (c == '"') ss << "\\\"";
                else if (c == '\\') ss << "\\\\";
                else if (c == '\n') ss << "\\n";
                else if (c == '\r') ss << "\\r";
                else if (c == '\t') ss << "\\t";
                else if (c < 32 || c == 127) { // Control characters
                    ss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(c) << std::dec << std::nouppercase;
                }
                else ss << static_cast<char>(c);
            }
            ss << "\",";
            ss << "\"status\":" << response.status_code << ",";
            ss << "\"size\":" << response.body.length() << ",";
            ss << "\"time\":" << response.response_time_ms << ",";
            ss << "\"bypass\":" << (bypass_success ? "true" : "false") << ",";
            ss << "\"reason\":\"" << bypass_reason << "\"";
            ss << "}";
            
            Logger::getInstance().info("Tested: " + variation + " -> " + 
                                     std::to_string(response.status_code) + 
                                     (bypass_success ? " (BYPASS!)" : ""));
        }
        
        ss << "],";
        ss << "\"summary\":{";
        ss << "\"total_tested\":" << total_tested << ",";
        ss << "\"successful_bypasses\":" << successful_bypasses << ",";
        ss << "\"failed_attempts\":" << (total_tested - successful_bypasses);
        ss << "}}";
        
        *result_json = string_to_c_str(ss.str());
        return BYPS_SUCCESS;
        
    } catch (const BypsException& e) {
        engine->last_error = e.what();
        Logger::getInstance().error(e.what());
        return static_cast<int>(e.code());
    } catch (const std::exception& e) {
        engine->last_error = e.what();
        Logger::getInstance().error(e.what());
        return BYPS_ERROR_UNKNOWN;
    }
}

void byps_string_free(char* str) {
    free(str);
}

const char* byps_version(void) {
    return "0.1.0";
}

const char* byps_last_error(BypsEngine* engine) {
    if (!engine) {
        return "Invalid engine pointer";
    }
    return engine->last_error.c_str();
}

} // extern "C"
