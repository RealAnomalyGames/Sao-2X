
#pragma once

#include <cstdint>
#include <string>

namespace sao {

class GraphicsDevice;

class ShaderProgram {
public:
    ShaderProgram() = default;

    bool is_linked() const noexcept;
    std::uint32_t handle() const noexcept;

    const std::string& log() const noexcept;

private:
    friend class GraphicsDevice;

    std::uint32_t handle_ = 0;
    bool linked_ = false;
    std::string log_;
};

} // namespace sao
