#pragma once

#ifdef _WIN32

#include "sao/platform/windows/window_win32.hpp"
#include "sao/rendering/graphics_backend.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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

    const std::string& last_error() const noexcept;

    bool make_current();
    void present();

    void set_viewport(int width, int height);
    void clear(float r, float g, float b, float a = 1.0f);

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

    void apply_render_state(const RenderState& state) override;

    void apply_texture_sampler(
        unsigned int texture_unit,
        const TextureSampler& sampler
    ) override;

    // GPU buffers.
    std::uint32_t create_buffer();
    void destroy_buffer(std::uint32_t buffer);

    bool upload_array_buffer(
        std::uint32_t buffer,
        const void* data,
        std::size_t size,
        bool dynamic = false
    );

    bool upload_element_buffer(
        std::uint32_t buffer,
        const void* data,
        std::size_t size,
        bool dynamic = false
    );

    // Vertex arrays.
    std::uint32_t create_vertex_array();
    void destroy_vertex_array(std::uint32_t vertex_array);
    void bind_vertex_array(std::uint32_t vertex_array);

    void configure_vertex_attribute(
        unsigned int index,
        int component_count,
        std::size_t stride,
        std::size_t offset
    );

    // Indexed drawing.
    void draw_indexed(std::uint32_t index_count);

    // GPU textures.
    std::uint32_t create_texture_2d();

    void destroy_texture(std::uint32_t texture);

    bool upload_texture_2d(
        std::uint32_t texture,
        int width,
        int height,
        const std::uint8_t* rgba_pixels
    );

    void bind_texture_2d(
        unsigned int texture_unit,
        std::uint32_t texture
    );

    // Shader uniforms.
    int uniform_location(
        std::uint32_t program,
        const char* name
    );

    void uniform_int(int location, int value);
    void uniform_float(int location, float value);

    void uniform_vec3(
        int location,
        float x,
        float y,
        float z
    );

    void uniform_mat4(
        int location,
        const float* column_major_matrix
    );

    const char* version_string() const noexcept;
    const char* renderer_string() const noexcept;

private:
    using GLProc = PROC;

    GLProc load_function(const char* name);

    bool create_context();
    bool load_functions();

    void set_error(const std::string& message);

    Win32Window& window_;

    HWND hwnd_ = nullptr;
    HDC device_context_ = nullptr;
    HGLRC context_ = nullptr;

    std::string last_error_;

    std::vector<unsigned int> sampler_objects_;

    bool initialized_ = false;

    struct Functions;
    Functions* gl_ = nullptr;
};

} // namespace sao

#endif
