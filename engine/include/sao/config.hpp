
#pragma once

#include <string>

#include "sao/logger.hpp"

namespace sao {

struct EngineConfig {
    std::string application_name = "Sao-2X Application";

    LogLevel log_level = LogLevel::Info;

    bool enable_console_logging = true;
    bool enable_file_logging = false;

    std::string log_file = "logs/sao-2x.log";

    unsigned int target_fps = 60;

    void validate() const;
};

}
