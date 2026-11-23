
#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace sao {

struct MemoryStats {
    std::size_t current_bytes = 0;
    std::size_t peak_bytes = 0;
    std::size_t total_allocated_bytes = 0;
    std::size_t allocation_count = 0;
    std::size_t active_allocations = 0;
};

class MemoryManager {
public:
    MemoryManager() = default;
    ~MemoryManager();

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    void* allocate(
        std::size_t size,
        std::size_t alignment = alignof(std::max_align_t),
        const std::string& label = {}
    );

    bool deallocate(void* memory) noexcept;

    MemoryStats get_stats() const noexcept;

    bool owns(const void* memory) const noexcept;

    void release_all() noexcept;

private:
    struct AllocationInfo {
        std::size_t size = 0;
        std::size_t alignment = alignof(std::max_align_t);
        std::string label;
    };

    mutable std::mutex mutex_;
    std::unordered_map<void*, AllocationInfo> allocations_;
    MemoryStats stats_;
};

}
