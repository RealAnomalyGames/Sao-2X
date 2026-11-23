
#include "sao/memory/memory_manager.hpp"

#include <new>
#include <stdexcept>

namespace sao {

MemoryManager::~MemoryManager() {
    release_all();
}

void* MemoryManager::allocate(
    std::size_t size,
    std::size_t alignment,
    const std::string& label
) {
    if (size == 0) {
        throw std::invalid_argument(
            "Cannot allocate zero bytes."
        );
    }

    if (alignment == 0 ||
        (alignment & (alignment - 1)) != 0) {
        throw std::invalid_argument(
            "Memory alignment must be a non-zero power of two."
        );
    }

    void* memory = nullptr;

    if (alignment <= alignof(std::max_align_t)) {
        memory = ::operator new(size);
    } else {
        memory = ::operator new(
            size,
            std::align_val_t(alignment)
        );
    }

    try {
        std::lock_guard<std::mutex> lock(mutex_);

        allocations_.emplace(
            memory,
            AllocationInfo{size, alignment, label}
        );

        stats_.current_bytes += size;
        stats_.total_allocated_bytes += size;
        ++stats_.allocation_count;
        ++stats_.active_allocations;

        if (stats_.current_bytes > stats_.peak_bytes) {
            stats_.peak_bytes = stats_.current_bytes;
        }
    } catch (...) {
        if (alignment <= alignof(std::max_align_t)) {
            ::operator delete(memory);
        } else {
            ::operator delete(
                memory,
                std::align_val_t(alignment)
            );
        }

        throw;
    }

    return memory;
}

bool MemoryManager::deallocate(void* memory) noexcept {
    if (memory == nullptr) {
        return true;
    }

    AllocationInfo info;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        const auto it = allocations_.find(memory);

        if (it == allocations_.end()) {
            return false;
        }

        info = std::move(it->second);
        allocations_.erase(it);

        stats_.current_bytes -= info.size;
        --stats_.active_allocations;
    }

    if (info.alignment <= alignof(std::max_align_t)) {
        ::operator delete(memory);
    } else {
        ::operator delete(
            memory,
            std::align_val_t(info.alignment)
        );
    }

    return true;
}

MemoryStats MemoryManager::get_stats() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

bool MemoryManager::owns(const void* memory) const noexcept {
    if (memory == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return allocations_.find(
        const_cast<void*>(memory)
    ) != allocations_.end();
}

void MemoryManager::release_all() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& entry : allocations_) {
        void* memory = entry.first;
        const AllocationInfo& info = entry.second;

        if (info.alignment <= alignof(std::max_align_t)) {
            ::operator delete(memory);
        } else {
            ::operator delete(
                memory,
                std::align_val_t(info.alignment)
            );
        }
    }

    allocations_.clear();
    stats_.current_bytes = 0;
    stats_.active_allocations = 0;
}

}
