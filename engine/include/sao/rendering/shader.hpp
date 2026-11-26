
#pragma once

#include <cstdint>
#include <string>

namespace sao {

enum class ShaderStage {
    Vertex,
    Fragment,
    Geometry,
    Compute
};

class Shader {
public:
    Shader() = default;

    Shader(ShaderStage stage, std::string source);

    void set_stage(ShaderStage stage) noexcept;
    void set_source(std::string source);

    ShaderStage stage() const noexcept;
    const std::string& source() const noexcept;

    bool is_compiled() const noexcept;
    std::uint32_t handle() const noexcept;

    const std::string& log() const noexcept;

private:
    friend class GraphicsDevice;

    ShaderStage stage_ = ShaderStage::Vertex;
    std::string source_;
    std::uint32_t handle_ = 0;
    bool compiled_ = false;
    std::string log_;
};

} // namespace sao
