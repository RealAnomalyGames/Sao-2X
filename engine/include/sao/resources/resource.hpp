
#pragma once

#include <string>

namespace sao {

enum class ResourceState {
    Unloaded,
    Loading,
    Loaded,
    Failed
};

class Resource {
public:
    virtual ~Resource() = default;
};

class AssetReference {
public:
    AssetReference() = default;
    explicit AssetReference(std::string path);

    const std::string& path() const noexcept;

    bool is_valid() const noexcept;

private:
    std::string path_;
};

}
