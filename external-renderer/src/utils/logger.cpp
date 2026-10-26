#include "logger.hpp"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace renderbridge {

// Global logger instance
static Logger* s_globalLogger = nullptr;

Logger& getLogger() {
    if (!s_globalLogger) {
        initializeLogger();
    }
    return *s_globalLogger;
}

void initializeLogger(const std::string& name, LogLevel level) {
    if (s_globalLogger) {
        delete s_globalLogger;
    }
    s_globalLogger = new Logger(name);
    s_globalLogger->setLevel(level);
    s_globalLogger->setOutput([](const std::string& msg) {
        std::cout << msg << std::endl;
    });
}

void shutdownLogger() {
    if (s_globalLogger) {
        delete s_globalLogger;
        s_globalLogger = nullptr;
    }
}

Logger::Logger(const std::string& name)
    : m_name(name), m_level(LogLevel::INFO), m_output(nullptr) {
}

Logger::~Logger() {
}

void Logger::setLevel(LogLevel level) {
    m_level = level;
}

LogLevel Logger::getLevel() const {
    return m_level;
}

void Logger::setOutput(OutputFunction output) {
    m_output = output;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_level) {
        return;
    }
    
    std::ostringstream oss;
    
    // Timestamp
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setw(3) << std::setfill('0') 
        << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    
    // Level
    oss << " [" << getLevelName(level) << "] ";
    
    // Logger name
    oss << "[" << m_name << "] ";
    
    // Message
    oss << message;
    
    // Output
    if (m_output) {
        m_output(oss.str());
    } else {
        std::cout << oss.str() << std::endl;
    }
}

std::string Logger::getLevelName(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::trace(const std::string& message) {
    log(LogLevel::TRACE, message);
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::FATAL, message);
}

} // namespace renderbridge
