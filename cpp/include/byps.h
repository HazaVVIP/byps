#ifndef BYPS_H
#define BYPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Opaque pointer to C++ engine
typedef struct BypsEngine BypsEngine;

// Error codes
typedef enum {
    BYPS_SUCCESS = 0,
    BYPS_ERROR_NETWORK = 1,
    BYPS_ERROR_PARSE = 2,
    BYPS_ERROR_TIMEOUT = 3,
    BYPS_ERROR_INVALID_URL = 4,
    BYPS_ERROR_OUT_OF_MEMORY = 5,
    BYPS_ERROR_UNKNOWN = 99
} BypsErrorCode;

// Create a new bypass engine
BypsEngine* byps_engine_new(void);

// Free the engine
void byps_engine_free(BypsEngine* engine);

// Perform a scan
// Returns error code, result is stored in result_json (caller must free with byps_string_free)
int byps_engine_scan(BypsEngine* engine,
                     const char* url,
                     const char* config_json,
                     char** result_json);

// Test a specific bypass technique
int byps_engine_test_technique(BypsEngine* engine,
                               const char* url,
                               const char* technique_name,
                               char** result_json);

// Detect WAF type
int byps_engine_detect_waf(BypsEngine* engine,
                           const char* url,
                           char** waf_info_json);

// Free a string allocated by the C++ library
void byps_string_free(char* str);

// Get version string
const char* byps_version(void);

// Get last error message
const char* byps_last_error(BypsEngine* engine);

#ifdef __cplusplus
}
#endif

#endif // BYPS_H
