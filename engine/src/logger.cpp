
#include "sao/logger.hpp"
#include "sao/error.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace sao {

Logger::~Logger() {
    shutdown();
}

void Logger::initialize(
    LogLevel minimum_level,
    bool console,
    bool file,
    const std::string& file_path
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        throw EngineError(
            ErrorCode::AlreadyInitialized,
            "Logger is already initialized."
        );
    }

    minimum_level_ = minimum_level;
    console_enabled_ = console;
    file_enabled_ = file;

    if (file_enabled_) {
        if (file_path.empty()) {
            throw EngineError(
                ErrorCode::InvalidConfiguration,
                "Log file path cannot be empty."
            );
        }

        std::filesystem::path path(file_path);

        if (!path.parent_path().empty()) {
            std::filesystem::create_directories(
                path.parent_path()
            );
        }

        file_.open(
            path,
            std::ios::out | std::ios::app
        );

        if (!file_.is_open()) {
            throw EngineError(
                ErrorCode::InitializationFailed,
                "Failed to open log file: " + file_path
            );
        }
    }

    initialized_ = true;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        return;
    }

    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }

    initialized_ = false;
}

std::string Logger::timestamp() const {
    const auto now = std::chrono::system_clock::now();

    const std::time_t time =
        std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};

#ifdef _WIN32
    localtime_s(&local_time, &time);
#else
    localtime_r(&time, &local_time);
#endif

    std::ostringstream stream;

    stream << std::put_time(
        &local_time,
        "%Y-%m-%d %H:%M:%S"
    );

    return stream.str();
}

const char* Logger::level_name(LogLevel level) const noexcept {
    switch (level) {
        case LogLevel::Trace:    return "TRACE";
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARNING";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        case LogLevel::Off:      return "OFF";
    }

    return "UNKNOWN";
}

void Logger::write(
    LogLevel level,
    const std::string& message
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_ || minimum_level_ == LogLevel::Off) {
        return;
    }

    if (level < minimum_level_ || level == LogLevel::Off) {
        return;
    }

    std::ostringstream stream;

    stream << "[" << timestamp() << "] "
           << "[" << level_name(level) << "] "
           << message;

    const std::string output = stream.str();

    if (console_enabled_) {
        if (level >= LogLevel::Error) {
            std::cerr << output << '\n';
        } else {
            std::cout << output << '\n';
        }
    }

    if (file_enabled_ && file_.is_open()) {
        file_ << output << '\n';
        file_.flush();
    }
}

void Logger::trace(const std::string& message) {
    write(LogLevel::Trace, message);
}

void Logger::debug(const std::string& message) {
    write(LogLevel::Debug, message);
}

void Logger::info(const std::string& message) {
    write(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) {
    write(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) {
    write(LogLevel::Error, message);
}

void Logger::critical(const std::string& message) {
    write(LogLevel::Critical, message);
}

}
