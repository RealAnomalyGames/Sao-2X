
#pragma once

#include "sao/rendering/render_state.hpp"
#include "sao/rendering/shader.hpp"
#include "sao/rendering/texture_sampler.hpp"

#include <cstdint>
#include <string>

namespace sao {

class GraphicsBackend {
public:
    virtual ~GraphicsBackend() = default;

    virtual const char* name() const noexcept = 0;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool compile_shader(
        ShaderStage stage,
        const std::string& source,
        std::uint32_t& shader_handle,
        std::string& log
    ) = 0;

    virtual void destroy_shader(std::uint32_t handle) = 0;

    virtual bool create_program(
        std::uint32_t& program_handle,
        std::string& log
    ) = 0;

    virtual bool attach_shader(
        std::uint32_t program_handle,
        std::uint32_t shader_handle,
        std::string& log
    ) = 0;

    virtual bool link_program(
        std::uint32_t program_handle,
        std::string& log
    ) = 0;

    virtual void destroy_program(std::uint32_t handle) = 0;

    virtual void use_program(std::uint32_t handle) = 0;

    virtual void apply_render_state(
        const RenderState& state
    ) = 0;

    virtual void apply_texture_sampler(
        unsigned int texture_unit,
        const TextureSampler& sampler
    ) = 0;
};

} // namespace sao
