
#include "sao/resources/resource.hpp"

#include <string>
#include <utility>

namespace sao {

AssetReference::AssetReference(std::string path)
    : path_(std::move(path)) {
}

const std::string& AssetReference::path() const noexcept {
    return path_;
}

bool AssetReference::is_valid() const noexcept {
    return !path_.empty();
}

}
