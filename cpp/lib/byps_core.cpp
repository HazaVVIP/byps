#include "byps.h"
#include "common/types.hpp"
#include "common/logger.hpp"
#include "common/error.hpp"
#include "common/utils.hpp"
#include "techniques/path_bypass.hpp"
#include "techniques/header_forge.hpp"
#include "techniques/encoding.hpp"
#include <cstring>
#include <memory>
#include <sstream>

using namespace byps;

// Internal structure to hold C++ objects
struct BypsEngine {
    std::unique_ptr<PathBypass> path_bypass;
    std::unique_ptr<HeaderForge> header_forge;
    std::unique_ptr<Encoding> encoding;
    std::string last_error;
    
    BypsEngine() 
        : path_bypass(std::make_unique<PathBypass>()),
          header_forge(std::make_unique<HeaderForge>()),
          encoding(std::make_unique<Encoding>()) {
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
