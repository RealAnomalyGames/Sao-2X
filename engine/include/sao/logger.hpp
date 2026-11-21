
#pragma once

#include <fstream>
#include <mutex>
#include <string>

namespace sao {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
    Off
};

class Logger {
public:
    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void initialize(
        LogLevel minimum_level,
        bool console,
        bool file,
        const std::string& file_path
    );

    void shutdown();

    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

    void write(LogLevel level, const std::string& message);

private:
    std::string timestamp() const;
    const char* level_name(LogLevel level) const noexcept;

    std::mutex mutex_;
    std::ofstream file_;

    LogLevel minimum_level_ = LogLevel::Info;

    bool console_enabled_ = true;
    bool file_enabled_ = false;
    bool initialized_ = false;
};

}
