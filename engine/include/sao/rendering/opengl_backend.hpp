
#pragma once

#include "sao/platform/windows/window_win32.hpp"
#include "sao/rendering/graphics_backend.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace sao {

class OpenGLBackend final : public GraphicsBackend {
public:
    explicit OpenGLBackend(Win32Window& window);
    ~OpenGLBackend() override;

    OpenGLBackend(const OpenGLBackend&) = delete;
    OpenGLBackend& operator=(const OpenGLBackend&) = delete;

    const char* name() const noexcept override;

    bool initialize() override;
    void shutdown() override;

    bool compile_shader(
        ShaderStage stage,
        const std::string& source,
        std::uint32_t& shader_handle,
        std::string& log
    ) override;

    void destroy_shader(std::uint32_t handle) override;

    bool create_program(
        std::uint32_t& program_handle,
        std::string& log
    ) override;

    bool attach_shader(
        std::uint32_t program_handle,
        std::uint32_t shader_handle,
        std::string& log
    ) override;

    bool link_program(
        std::uint32_t program_handle,
        std::string& log
    ) override;

    void destroy_program(std::uint32_t handle) override;
    void use_program(std::uint32_t handle) override;

    void apply_render_state(
        const RenderState& state
    ) override;

    void apply_texture_sampler(
        unsigned int texture_unit,
        const TextureSampler& sampler
    ) override;

    bool is_available() const noexcept;

    const std::string& renderer_name() const noexcept;
    const std::string& version_string() const noexcept;

    void present();

private:
    struct GLFunctions;

    bool create_context(std::string& error);
    bool load_functions(std::string& error);
    void destroy_context() noexcept;

    Win32Window& window_;

    HDC device_context_ = nullptr;
    HGLRC rendering_context_ = nullptr;

    std::unique_ptr<GLFunctions> gl_;

    bool initialized_ = false;

    std::string renderer_name_;
    std::string version_string_;

    std::unordered_map<unsigned int, unsigned int> sampler_objects_;
};

} // namespace sao
