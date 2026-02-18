#ifndef BYPS_COMMON_LOGGER_HPP
#define BYPS_COMMON_LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

namespace byps {

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLevel(LogLevel level);
    void setOutputFile(const std::string& filename);
    void enableConsole(bool enable);
    
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    void log(LogLevel level, const std::string& message);
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void writeLog(LogLevel level, const std::string& message);
    std::string levelToString(LogLevel level) const;
    std::string getCurrentTimestamp() const;
    
    LogLevel current_level_;
    bool console_enabled_;
    std::ofstream file_stream_;
    std::mutex mutex_;
};

} // namespace byps

#endif // BYPS_COMMON_LOGGER_HPP
