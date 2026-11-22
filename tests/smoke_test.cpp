#include "sao/platform.hpp"
#include "sao/platform_info.hpp"
#include "sao/filesystem.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

void test_platform_detection() {
    const sao::PlatformInfo info = sao::PlatformDetector::detect();

    assert(info.type != sao::PlatformType::Unknown);
    assert(!info.name.empty());
    assert(!info.architecture_name.empty());

    std::cout << "Detected platform: "
              << info.name << " "
              << info.version << '\n';

    std::cout << "Architecture: "
              << info.architecture_name << '\n';
}

void test_platform_lifecycle() {
    sao::Platform platform;

    assert(!platform.is_initialized());

    platform.initialize();

    assert(platform.is_initialized());
    assert(platform.info().type != sao::PlatformType::Unknown);

    platform.shutdown();

    assert(!platform.is_initialized());
}


#include <stdexcept>

void test_filesystem() {
    const auto test_directory =
        sao::FileSystem::current_directory() / "sao2x_test";

    const auto test_file =
        test_directory / "test.txt";

    // Remove leftovers from previous runs.
    if (sao::FileSystem::exists(test_directory)) {
        if (!sao::FileSystem::remove_directory(test_directory)) {
            throw std::runtime_error(
                "Could not remove previous filesystem test directory."
            );
        }
    }

    // Create the directory.
    const bool created =
        sao::FileSystem::create_directories(test_directory);

    if (!created || !sao::FileSystem::is_directory(test_directory)) {
        throw std::runtime_error(
            "Filesystem test directory could not be created: " +
            test_directory.string()
        );
    }

    // Write a file.
    sao::FileSystem::write_text(
        test_file,
        "Sao-2X platform test"
    );

    if (!sao::FileSystem::is_file(test_file)) {
        throw std::runtime_error(
            "Filesystem test file was not created."
        );
    }

    // Read the file.
    if (sao::FileSystem::read_text(test_file) !=
        "Sao-2X platform test") {
        throw std::runtime_error(
            "Filesystem text reading test failed."
        );
    }

    // Append text.
    sao::FileSystem::append_text(test_file, " passed");

    if (sao::FileSystem::read_text(test_file) !=
        "Sao-2X platform test passed") {
        throw std::runtime_error(
            "Filesystem append test failed."
        );
    }

    // Remove the file.
    if (!sao::FileSystem::remove_file(test_file)) {
        throw std::runtime_error(
            "Could not remove filesystem test file."
        );
    }

    // Remove the directory.
    if (!sao::FileSystem::remove_directory(test_directory)) {
        throw std::runtime_error(
            "Could not remove filesystem test directory."
        );
    }
}

int main() {
    test_platform_detection();
    test_platform_lifecycle();
    test_filesystem();

    return 0;
}
