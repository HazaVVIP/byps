#include "common/error.hpp"
#include <sstream>

namespace byps {

BypsException::BypsException(ErrorCode code, const std::string& message)
    : code_(code), message_(message), details_("") {
}

BypsException::BypsException(ErrorCode code, const std::string& message, const std::string& details)
    : code_(code), message_(message), details_(details) {
}

const char* BypsException::what() const noexcept {
    if (full_message_.empty()) {
        std::stringstream ss;
        ss << "[Error " << static_cast<int>(code_) << "] " << message_;
        if (!details_.empty()) {
            ss << " (" << details_ << ")";
        }
        full_message_ = ss.str();
    }
    return full_message_.c_str();
}

} // namespace byps
