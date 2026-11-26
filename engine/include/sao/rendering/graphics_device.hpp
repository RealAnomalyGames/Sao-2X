
#pragma once

#include "sao/rendering/graphics_backend.hpp"
#include "sao/rendering/shader.hpp"
#include "sao/rendering/shader_program.hpp"

#include <memory>
#include <vector>

namespace sao {

class GraphicsDevice {
public:
    GraphicsDevice() = default;
    ~GraphicsDevice();

    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;

    bool initialize(std::unique_ptr<GraphicsBackend> backend);

    void shutdown();

    bool is_initialized() const noexcept;

    const char* backend_name() const noexcept;

    bool compile_shader(Shader& shader);

    bool link_program(
        ShaderProgram& program,
        const std::vector<const Shader*>& shaders
    );

    void use_program(const ShaderProgram* program);

    void destroy_shader(Shader& shader);
    void destroy_program(ShaderProgram& program);

    void set_render_state(const RenderState& state);

    void set_texture_sampler(
        unsigned int texture_unit,
        const TextureSampler& sampler
    );

private:
    std::unique_ptr<GraphicsBackend> backend_;
    bool initialized_ = false;
};

} // namespace sao
