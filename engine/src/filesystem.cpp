
#include "sao/filesystem.hpp"
#include "sao/error.hpp"

#include <fstream>
#include <iterator>
#include <system_error>

#if defined(_WIN32)
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
#endif

namespace sao {

bool FileSystem::exists(const Path& path) {
    std::error_code error;
    return std::filesystem::exists(path, error) && !error;
}

bool FileSystem::is_file(const Path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path, error) && !error;
}

bool FileSystem::is_directory(const Path& path) {
    std::error_code error;
    return std::filesystem::is_directory(path, error) && !error;
}

bool FileSystem::create_directory(const Path& path) {
    std::error_code error;

    if (std::filesystem::is_directory(path, error)) {
        return true;
    }

    error.clear();

    const bool created = std::filesystem::create_directory(path, error);

    if (error) {
        return false;
    }

    return created || std::filesystem::is_directory(path, error);
}

bool FileSystem::create_directories(const Path& path) {
    std::error_code error;

    if (std::filesystem::is_directory(path, error)) {
        return true;
    }

    error.clear();

    const bool created =
        std::filesystem::create_directories(path, error);

    if (error) {
        return false;
    }

    return created || std::filesystem::is_directory(path, error);
}

bool FileSystem::remove_file(const Path& path) {
    std::error_code error;

    if (!std::filesystem::is_regular_file(path, error) || error) {
        return false;
    }

    return std::filesystem::remove(path, error) && !error;
}

bool FileSystem::remove_directory(const Path& path) {
    std::error_code error;

    if (!std::filesystem::is_directory(path, error) || error) {
        return false;
    }

    return std::filesystem::remove_all(path, error) > 0 && !error;
}

std::vector<FileSystem::Path> FileSystem::list_directory(
    const Path& path
) {
    if (!is_directory(path)) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Cannot list directory: " + path.string()
        );
    }

    std::vector<Path> entries;
    std::error_code error;

    for (std::filesystem::directory_iterator iterator(path, error);
         !error && iterator != std::filesystem::directory_iterator();
         iterator.increment(error)) {
        entries.push_back(iterator->path());
    }

    if (error) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Failed to read directory: " + path.string()
        );
    }

    return entries;
}

std::string FileSystem::read_text(const Path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Failed to open file for reading: " + path.string()
        );
    }

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

void FileSystem::write_text(
    const Path& path,
    const std::string& content
) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Failed to open file for writing: " + path.string()
        );
    }

    file.write(content.data(), static_cast<std::streamsize>(content.size()));

    if (!file) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Failed to write file: " + path.string()
        );
    }
}

void FileSystem::append_text(
    const Path& path,
    const std::string& content
) {
    std::ofstream file(path, std::ios::binary | std::ios::app);

    if (!file) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Failed to open file for appending: " + path.string()
        );
    }

    file.write(content.data(), static_cast<std::streamsize>(content.size()));

    if (!file) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Failed to append to file: " + path.string()
        );
    }
}

FileSystem::Path FileSystem::current_directory() {
    std::error_code error;
    Path result = std::filesystem::current_path(error);

    if (error) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Unable to determine current directory."
        );
    }

    return result;
}

FileSystem::Path FileSystem::executable_directory() {
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH]{};

    const DWORD length = GetModuleFileNameW(
        nullptr,
        buffer,
        MAX_PATH
    );

    if (length == 0 || length >= MAX_PATH) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Unable to determine executable directory."
        );
    }

    return Path(buffer).parent_path();
#else
    // A portable executable-path implementation will be supplied
    // by each additional platform backend.
    return current_directory();
#endif
}

FileSystem::Path FileSystem::normalize(const Path& path) {
    std::error_code error;
    Path result = std::filesystem::weakly_canonical(path, error);

    if (error) {
        return path.lexically_normal();
    }

    return result;
}

FileSystem::Path FileSystem::combine(
    const Path& first,
    const Path& second
) {
    return (first / second).lexically_normal();
}

} // namespace sao
