
#pragma once

#include "sao/resources/resource.hpp"
#include "sao/resources/resource_handle.hpp"

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sao {

template <typename T>
class ResourceManager {
    static_assert(
        std::is_base_of<Resource, T>::value,
        "Managed resource types must derive from sao::Resource."
    );

public:
    using Handle = ResourceHandle<T>;
    using ResourcePtr = std::shared_ptr<T>;
    using Loader = std::function<ResourcePtr(const AssetReference&)>;

    explicit ResourceManager(Loader loader)
        : loader_(std::move(loader)) {
        if (!loader_) {
            throw std::invalid_argument(
                "ResourceManager requires a valid loader."
            );
        }
    }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    Handle load(const AssetReference& asset) {
        if (!asset.is_valid()) {
            throw std::invalid_argument(
                "Cannot load an asset with an empty path."
            );
        }

        std::lock_guard<std::mutex> lock(mutex_);

        const std::string& path = asset.path();

        const auto existing = cache_.find(path);

        if (existing != cache_.end()) {
            const Slot& slot = slots_[existing->second];

            return Handle(
                existing->second,
                slot.generation
            );
        }

        ResourcePtr resource = loader_(asset);

        if (!resource) {
            throw std::runtime_error(
                "Resource loader returned an empty resource: " + path
            );
        }

        std::uint32_t index;

        if (!free_slots_.empty()) {
            index = free_slots_.back();
            free_slots_.pop_back();
        } else {
            if (slots_.size() >= Handle::InvalidIndex) {
                throw std::runtime_error(
                    "ResourceManager has exhausted its handle indices."
                );
            }

            index = static_cast<std::uint32_t>(slots_.size());
            slots_.emplace_back();
        }

        Slot& slot = slots_[index];

        slot.path = path;
        slot.resource = std::move(resource);
        slot.state = ResourceState::Loaded;

        try {
            cache_.emplace(path, index);
        } catch (...) {
            slot.resource.reset();
            slot.path.clear();
            slot.state = ResourceState::Unloaded;
            free_slots_.push_back(index);
            throw;
        }

        return Handle(index, slot.generation);
    }

    ResourcePtr get(Handle handle) const {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!is_valid_unlocked(handle)) {
            return nullptr;
        }

        return slots_[handle.index()].resource;
    }

    ResourceState get_state(Handle handle) const {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!is_valid_unlocked(handle)) {
            return ResourceState::Unloaded;
        }

        return slots_[handle.index()].state;
    }

    bool is_loaded(Handle handle) const {
        return get_state(handle) == ResourceState::Loaded;
    }

    bool unload(Handle handle) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!is_valid_unlocked(handle)) {
            return false;
        }

        Slot& slot = slots_[handle.index()];

        cache_.erase(slot.path);

        slot.resource.reset();
        slot.path.clear();
        slot.state = ResourceState::Unloaded;

        ++slot.generation;

        if (slot.generation == 0) {
            ++slot.generation;
        }

        free_slots_.push_back(handle.index());

        return true;
    }

    void unload_all() {
        std::lock_guard<std::mutex> lock(mutex_);

        cache_.clear();
        free_slots_.clear();

        for (std::uint32_t i = 0; i < slots_.size(); ++i) {
            Slot& slot = slots_[i];

            slot.resource.reset();
            slot.path.clear();
            slot.state = ResourceState::Unloaded;

            ++slot.generation;

            if (slot.generation == 0) {
                ++slot.generation;
            }

            free_slots_.push_back(i);
        }
    }

    std::size_t loaded_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }

private:
    struct Slot {
        ResourcePtr resource;
        std::string path;
        ResourceState state = ResourceState::Unloaded;
        std::uint32_t generation = 1;
    };

    bool is_valid_unlocked(Handle handle) const {
        if (!handle.is_valid() ||
            handle.index() >= slots_.size()) {
            return false;
        }

        const Slot& slot = slots_[handle.index()];

        return slot.generation == handle.generation() &&
               slot.state == ResourceState::Loaded &&
               static_cast<bool>(slot.resource);
    }

    Loader loader_;

    mutable std::mutex mutex_;
    std::vector<Slot> slots_;
    std::vector<std::uint32_t> free_slots_;
    std::unordered_map<std::string, std::uint32_t> cache_;
};

}
