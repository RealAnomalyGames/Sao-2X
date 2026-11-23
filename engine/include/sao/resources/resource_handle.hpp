
#pragma once

#include <cstdint>
#include <limits>

namespace sao {

template <typename T>
class ResourceHandle {
public:
    static constexpr std::uint32_t InvalidIndex =
        std::numeric_limits<std::uint32_t>::max();

    ResourceHandle() = default;

    ResourceHandle(
        std::uint32_t index,
        std::uint32_t generation
    ) noexcept
        : index_(index),
          generation_(generation) {
    }

    bool is_valid() const noexcept {
        return index_ != InvalidIndex;
    }

    explicit operator bool() const noexcept {
        return is_valid();
    }

    std::uint32_t index() const noexcept {
        return index_;
    }

    std::uint32_t generation() const noexcept {
        return generation_;
    }

    bool operator==(const ResourceHandle& other) const noexcept {
        return index_ == other.index_ &&
               generation_ == other.generation_;
    }

    bool operator!=(const ResourceHandle& other) const noexcept {
        return !(*this == other);
    }

private:
    std::uint32_t index_ = InvalidIndex;
    std::uint32_t generation_ = 0;
};

}
