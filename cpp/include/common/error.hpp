#ifndef BYPS_COMMON_ERROR_HPP
#define BYPS_COMMON_ERROR_HPP

#include <exception>
#include <string>

namespace byps {

enum class ErrorCode {
    SUCCESS = 0,
    NETWORK_ERROR = 1,
    PARSE_ERROR = 2,
    TIMEOUT = 3,
    WAF_DETECTED = 4,
    INVALID_URL = 5,
    CONNECTION_FAILED = 6,
    SSL_ERROR = 7,
    INVALID_RESPONSE = 8,
    OUT_OF_MEMORY = 9,
    UNKNOWN_ERROR = 99
};

class BypsException : public std::exception {
public:
    BypsException(ErrorCode code, const std::string& message);
    BypsException(ErrorCode code, const std::string& message, const std::string& details);
    
    const char* what() const noexcept override;
    ErrorCode code() const noexcept { return code_; }
    const std::string& details() const noexcept { return details_; }
    
private:
    ErrorCode code_;
    std::string message_;
    std::string details_;
    mutable std::string full_message_;
};

class NetworkException : public BypsException {
public:
    NetworkException(const std::string& message) 
        : BypsException(ErrorCode::NETWORK_ERROR, message) {}
};

class ParseException : public BypsException {
public:
    ParseException(const std::string& message)
        : BypsException(ErrorCode::PARSE_ERROR, message) {}
};

class TimeoutException : public BypsException {
public:
    TimeoutException(const std::string& message)
        : BypsException(ErrorCode::TIMEOUT, message) {}
};

} // namespace byps

#endif // BYPS_COMMON_ERROR_HPP
