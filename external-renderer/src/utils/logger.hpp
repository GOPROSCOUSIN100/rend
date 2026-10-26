#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <functional>

namespace renderbridge {

/**
 * @enum LogLevel
 * @brief Logging levels
 */
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @class Logger
 * @brief Simple logging utility
 */
class Logger {
public:
    /**
     * @brief Constructor
     * @param name Name of the logger
     */
    explicit Logger(const std::string& name);
    
    /**
     * @brief Destructor
     */
    ~Logger();
    
    /**
     * @brief Set the log level
     * @param level Log level
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief Get the log level
     * @return Log level
     */
    LogLevel getLevel() const;
    
    /**
     * @brief Set the output stream
     * @param output Output stream function
     */
    using OutputFunction = std::function<void(const std::string&)>;
    void setOutput(OutputFunction output);
    
    /**
     * @brief Log a message
     * @param level Log level
     * @param message Message
     */
    void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Log a formatted message
     * @param level Log level
     * @param format Format string
     * @param ... Arguments
     */
    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args... args) {
        if (level >= m_level) {
            std::ostringstream oss;
            formatMessage(oss, format, args...);
            log(level, oss.str());
        }
    }
    
    /**
     * @brief Trace level log
     * @param message Message
     */
    void trace(const std::string& message);
    
    /**
     * @brief Debug level log
     * @param message Message
     */
    void debug(const std::string& message);
    
    /**
     * @brief Info level log
     * @param message Message
     */
    void info(const std::string& message);
    
    /**
     * @brief Warning level log
     * @param message Message
     */
    void warn(const std::string& message);
    
    /**
     * @brief Error level log
     * @param message Message
     */
    void error(const std::string& message);
    
    /**
     * @brief Fatal level log
     * @param message Message
     */
    void fatal(const std::string& message);
    
    /**
     * @brief Trace level formatted log
     * @param format Format string
     * @param ... Arguments
     */
    template<typename... Args>
    void trace(const std::string& format, Args... args) {
        log(LogLevel::TRACE, format, args...);
    }
    
    /**
     * @brief Debug level formatted log
     * @param format Format string
     * @param ... Arguments
     */
    template<typename... Args>
    void debug(const std::string& format, Args... args) {
        log(LogLevel::DEBUG, format, args...);
    }
    
    /**
     * @brief Info level formatted log
     * @param format Format string
     * @param ... Arguments
     */
    template<typename... Args>
    void info(const std::string& format, Args... args) {
        log(LogLevel::INFO, format, args...);
    }
    
    /**
     * @brief Warning level formatted log
     * @param format Format string
     * @param ... Arguments
     */
    template<typename... Args>
    void warn(const std::string& format, Args... args) {
        log(LogLevel::WARN, format, args...);
    }
    
    /**
     * @brief Error level formatted log
     * @param format Format string
     * @param ... Arguments
     */
    template<typename... Args>
    void error(const std::string& format, Args... args) {
        log(LogLevel::ERROR, format, args...);
    }
    
    /**
     * @brief Fatal level formatted log
     * @param format Format string
     * @param ... Arguments
     */
    template<typename... Args>
    void fatal(const std::string& format, Args... args) {
        log(LogLevel::FATAL, format, args...);
    }

private:
    std::string m_name;
    LogLevel m_level;
    OutputFunction m_output;
    
    /**
     * @brief Get the log level name
     * @param level Log level
     * @return Level name
     */
    static std::string getLevelName(LogLevel level);
    
    /**
     * @brief Format a message
     * @param oss Output string stream
     * @param format Format string
     * @param args Arguments
     */
    template<typename... Args>
    static void formatMessage(std::ostringstream& oss, const std::string& format, Args... args);
};

/**
 * @brief Format a message with arguments
 * @param oss Output string stream
 * @param format Format string
 * @param args Arguments
 */
template<typename... Args>
void Logger::formatMessage(std::ostringstream& oss, const std::string& format, Args... args) {
    // Simple format implementation (no format specifiers)
    oss << format;
    // Note: This is a simplified implementation. In a real implementation,
    // you would use a proper format library or implement format specifiers.
}

/**
 * @brief Global logger instance
 * @return Reference to the global logger
 */
Logger& getLogger();

/**
 * @brief Initialize the global logger
 * @param name Name of the logger
 * @param level Log level
 */
void initializeLogger(const std::string& name = "RenderBridge", LogLevel level = LogLevel::INFO);

/**
 * @brief Shutdown the global logger
 */
void shutdownLogger();

/**
 * @brief Convenience macros for logging
 */
#define RB_LOG_TRACE(msg) renderbridge::getLogger().trace(msg)
#define RB_LOG_DEBUG(msg) renderbridge::getLogger().debug(msg)
#define RB_LOG_INFO(msg) renderbridge::getLogger().info(msg)
#define RB_LOG_WARN(msg) renderbridge::getLogger().warn(msg)
#define RB_LOG_ERROR(msg) renderbridge::getLogger().error(msg)
#define RB_LOG_FATAL(msg) renderbridge::getLogger().fatal(msg)

/**
 * @brief Convenience macros for formatted logging
 */
#define RB_LOG_TRACE_FMT(format, ...) renderbridge::getLogger().trace(format, ##__VA_ARGS__)
#define RB_LOG_DEBUG_FMT(format, ...) renderbridge::getLogger().debug(format, ##__VA_ARGS__)
#define RB_LOG_INFO_FMT(format, ...) renderbridge::getLogger().info(format, ##__VA_ARGS__)
#define RB_LOG_WARN_FMT(format, ...) renderbridge::getLogger().warn(format, ##__VA_ARGS__)
#define RB_LOG_ERROR_FMT(format, ...) renderbridge::getLogger().error(format, ##__VA_ARGS__)
#define RB_LOG_FATAL_FMT(format, ...) renderbridge::getLogger().fatal(format, ##__VA_ARGS__)

} // namespace renderbridge
