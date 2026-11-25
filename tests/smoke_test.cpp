#include "sao/platform.hpp"
#include "sao/platform_info.hpp"
#include "sao/filesystem.hpp"
#include "sao/memory/memory_manager.hpp"
#include "sao/resources/resource.hpp"
#include "sao/resources/resource_manager.hpp"

#include <memory>
#include <stdexcept>
#include <cassert>
#include <filesystem>
#include <iostream>

class TestResource : public sao::Resource {
public:
    explicit TestResource(int resource_value)
        : value(resource_value) {
    }

    int value;
};

void test_resource_manager() {
    int load_count = 0;

    sao::ResourceManager<TestResource> manager(
        [&load_count](const sao::AssetReference& asset) {
            ++load_count;

            if (asset.path() == "assets/test.asset") {
                return std::make_shared<TestResource>(42);
            }

            return std::shared_ptr<TestResource>();
        }
    );

    const sao::AssetReference asset("assets/test.asset");

    const auto first = manager.load(asset);

    if (!first.is_valid()) {
        throw std::runtime_error(
            "Resource manager returned an invalid handle."
        );
    }

    if (!manager.is_loaded(first)) {
        throw std::runtime_error(
            "Resource was not marked as loaded."
        );
    }

    auto resource = manager.get(first);

    if (!resource || resource->value != 42) {
        throw std::runtime_error(
            "Resource retrieval failed."
        );
    }

    const auto second = manager.load(asset);

    if (first != second || load_count != 1) {
        throw std::runtime_error(
            "Resource caching failed."
        );
    }

    if (manager.loaded_count() != 1) {
        throw std::runtime_error(
            "Resource count is incorrect."
        );
    }

    if (!manager.unload(first)) {
        throw std::runtime_error(
            "Resource unloading failed."
        );
    }

    if (manager.get(first)) {
        throw std::runtime_error(
            "An unloaded resource handle remained valid."
        );
    }

    if (manager.loaded_count() != 0) {
        throw std::runtime_error(
            "Resource count was not updated after unloading."
        );
    }

    const auto third = manager.load(asset);

    if (!third.is_valid() || third == first) {
        throw std::runtime_error(
            "Resource handle generation tracking failed."
        );
    }

    if (load_count != 2) {
        throw std::runtime_error(
            "Resource was not loaded again after unloading."
        );
    }

    manager.unload_all();

    if (manager.loaded_count() != 0 ||
        manager.get(third)) {
        throw std::runtime_error(
            "ResourceManager::unload_all() failed."
        );
    }
}

void test_memory_manager() {
    sao::MemoryManager memory;

    void* first = memory.allocate(256, 16, "First test allocation");
    void* second = memory.allocate(512, 32, "Second test allocation");

    if (!memory.owns(first) || !memory.owns(second)) {
        throw std::runtime_error(
            "Memory manager ownership tracking failed."
        );
    }

    sao::MemoryStats stats = memory.get_stats();

    if (stats.current_bytes != 768 ||
        stats.active_allocations != 2 ||
        stats.allocation_count != 2) {
        throw std::runtime_error(
            "Memory allocation statistics are incorrect."
        );
    }

    if (!memory.deallocate(first)) {
        throw std::runtime_error(
            "Memory deallocation failed."
        );
    }

    stats = memory.get_stats();

    if (stats.current_bytes != 512 ||
        stats.active_allocations != 1) {
        throw std::runtime_error(
            "Memory statistics were not updated after deallocation."
        );
    }

    if (memory.deallocate(first)) {
        throw std::runtime_error(
            "Memory manager accepted a duplicate deallocation."
        );
    }

    if (!memory.deallocate(second)) {
        throw std::runtime_error(
            "Second memory deallocation failed."
        );
    }

    stats = memory.get_stats();

    if (stats.current_bytes != 0 ||
        stats.active_allocations != 0) {
        throw std::runtime_error(
            "Memory manager did not release all test allocations."
        );
    }

    if (stats.peak_bytes != 768) {
        throw std::runtime_error(
            "Memory peak allocation tracking failed."
        );
    }
}

void test_platform_detection() {
    const sao::PlatformInfo info =
        sao::PlatformDetector::detect();

    if (info.type == sao::PlatformType::Unknown) {
        throw std::runtime_error(
            "Platform detection returned Unknown."
        );
    }

    if (info.name.empty()) {
        throw std::runtime_error(
            "Platform name is empty."
        );
    }

    if (info.architecture_name.empty()) {
        throw std::runtime_error(
            "Architecture name is empty."
        );
    }

    std::cout << "Detected platform: "
              << info.name << " "
              << info.version << '\n';

    std::cout << "Architecture: "
              << info.architecture_name << '\n';
}

void test_platform_lifecycle() {
    sao::Platform platform;

    !platform.is_initialized();

    platform.initialize();

    platform.is_initialized();
    platform.info().type != sao::PlatformType::Unknown;

    platform.shutdown();

    !platform.is_initialized();
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
    test_memory_manager();
    test_resource_manager();

    return 0;
}
