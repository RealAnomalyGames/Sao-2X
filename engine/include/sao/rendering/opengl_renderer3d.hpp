#pragma once

#ifdef _WIN32

#include "sao/platform/windows/window_win32.hpp"
#include "sao/rendering/camera3d.hpp"
#include "sao/rendering/light.hpp"
#include "sao/rendering/material.hpp"
#include "sao/rendering/math3d.hpp"
#include "sao/rendering/mesh.hpp"
#include "sao/rendering/model.hpp"
#include "sao/rendering/opengl_backend.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace sao {

class OpenGLRenderer3D {
public:
    explicit OpenGLRenderer3D(Win32Window& window);
    ~OpenGLRenderer3D();

    OpenGLRenderer3D(const OpenGLRenderer3D&) = delete;
    OpenGLRenderer3D& operator=(const OpenGLRenderer3D&) = delete;

    bool initialize();
    void shutdown();

    bool is_initialized() const noexcept;

    const std::string& last_error() const noexcept;

    const char* graphics_version() const noexcept;
    const char* graphics_renderer() const noexcept;

    void resize(int width, int height);

    void add_light(const Light3D& light);
    void clear_lights();

    void begin_frame(
        const Camera3D& camera,
        Color3 clear_color = {0.08f, 0.09f, 0.14f}
    );

    void draw_mesh(
        const Mesh& mesh,
        const Transform3D& transform,
        const Material& material
    );

    void draw_model(
        const Model& model,
        const Transform3D& transform
    );

    void end_frame();

private:
    struct GPUVertex {
        float position[3];
        float normal[3];
        float uv[2];
    };

    bool create_shaders();
    bool create_gpu_objects();

    void destroy_gpu_objects();

    void upload_matrix(
        const char* uniform,
        const Matrix4& matrix
    );

    void upload_lighting();

    static void to_column_major(
        const Matrix4& source,
        float destination[16]
    );

    Win32Window& window_;

    std::unique_ptr<OpenGLBackend> backend_;

    std::uint32_t vertex_shader_ = 0;
    std::uint32_t fragment_shader_ = 0;
    std::uint32_t program_ = 0;

    std::uint32_t vertex_array_ = 0;
    std::uint32_t vertex_buffer_ = 0;
    std::uint32_t index_buffer_ = 0;

    Camera3D camera_;

    std::vector<Light3D> lights_;

    int width_ = 0;
    int height_ = 0;

    bool initialized_ = false;
    bool frame_active_ = false;

    std::string last_error_;
};

} // namespace sao

#endif
