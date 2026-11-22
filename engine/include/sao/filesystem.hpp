
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace sao {

class FileSystem {
public:
    using Path = std::filesystem::path;

    static bool exists(const Path& path);
    static bool is_file(const Path& path);
    static bool is_directory(const Path& path);

    static bool create_directory(const Path& path);
    static bool create_directories(const Path& path);

    static bool remove_file(const Path& path);
    static bool remove_directory(const Path& path);

    static std::vector<Path> list_directory(const Path& path);

    static std::string read_text(const Path& path);
    static void write_text(const Path& path, const std::string& content);
    static void append_text(const Path& path, const std::string& content);

    static Path current_directory();
    static Path executable_directory();

    static Path normalize(const Path& path);
    static Path combine(const Path& first, const Path& second);
};

} // namespace sao
