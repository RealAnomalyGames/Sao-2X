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

void test_filesystem() {
    const auto test_directory =
        sao::FileSystem::current_directory() / "sao2x_test";

    const auto test_file =
        test_directory / "test.txt";

    assert(sao::FileSystem::create_directories(test_directory));
    assert(sao::FileSystem::is_directory(test_directory));

    sao::FileSystem::write_text(test_file, "Sao-2X platform test");

    assert(sao::FileSystem::exists(test_file));
    assert(sao::FileSystem::is_file(test_file));

    const std::string contents =
        sao::FileSystem::read_text(test_file);

    assert(contents == "Sao-2X platform test");

    sao::FileSystem::append_text(test_file, " passed");

    assert(
        sao::FileSystem::read_text(test_file) ==
        "Sao-2X platform test passed"
    );

    assert(sao::FileSystem::remove_file(test_file));
    assert(sao::FileSystem::remove_directory(test_directory));
}

int main() {
    test_platform_detection();
    test_platform_lifecycle();
    test_filesystem();

    return 0;
}
