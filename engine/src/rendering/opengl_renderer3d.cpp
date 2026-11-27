#ifdef _WIN32

#include "sao/rendering/opengl_renderer3d.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace sao {

namespace {

constexpr const char* VERTEX_SHADER_SOURCE = R"GLSL(
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec2 vUV;

void main() {
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);

    vWorldPosition = worldPosition.xyz;

    vWorldNormal = normalize(
        mat3(transpose(inverse(uModel))) * aNormal
    );

    vUV = aUV;

    gl_Position =
        uProjection *
        uView *
        worldPosition;
}
)GLSL";

constexpr const char* FRAGMENT_SHADER_SOURCE = R"GLSL(
#version 330 core

struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range;
};

uniform Light uLights[8];
uniform int uLightCount;

uniform vec3 uCameraPosition;

uniform vec3 uDiffuse;
uniform vec3 uAmbient;
uniform vec3 uSpecular;

uniform float uShininess;
uniform float uOpacity;

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vUV;

out vec4 FragColor;

void main() {
    vec3 normal = normalize(vWorldNormal);
    vec3 viewDirection =
        normalize(uCameraPosition - vWorldPosition);

    vec3 result = uAmbient * uDiffuse;

    for (int i = 0; i < uLightCount; ++i) {
        vec3 lightDirection;
        float attenuation = 1.0;

        if (uLights[i].type == 0) {
            lightDirection =
                normalize(-uLights[i].direction);
        } else {
            vec3 difference =
                uLights[i].position - vWorldPosition;

            float distanceToLight = length(difference);

            lightDirection =
                normalize(difference);

            float range = max(uLights[i].range, 0.001);

            float factor =
                clamp(1.0 - distanceToLight / range, 0.0, 1.0);

            attenuation = factor * factor;
        }

        float diffuseFactor =
            max(dot(normal, lightDirection), 0.0);

        vec3 reflected =
            reflect(-lightDirection, normal);

        float specularFactor = 0.0;

        if (diffuseFactor > 0.0) {
            specularFactor = pow(
                max(dot(viewDirection, reflected), 0.0),
                max(uShininess, 1.0)
            );
        }

        vec3 lightColor =
            uLights[i].color *
            uLights[i].intensity *
            attenuation;

        vec3 diffuseContribution =
            uDiffuse * diffuseFactor * lightColor;

        vec3 specularContribution =
            uSpecular * specularFactor * lightColor;

        result +=
            diffuseContribution +
            specularContribution;
    }

    FragColor = vec4(
        clamp(result, 0.0, 1.0),
        clamp(uOpacity, 0.0, 1.0)
    );
}
)GLSL";

} // namespace

OpenGLRenderer3D::OpenGLRenderer3D(Win32Window& window)
    : window_(window) {
}

OpenGLRenderer3D::~OpenGLRenderer3D() {
    shutdown();
}

bool OpenGLRenderer3D::initialize() {
    if (initialized_) {
        return true;
    }

    last_error_.clear();

    backend_ = std::make_unique<OpenGLBackend>(window_);

    if (!backend_->initialize()) {
        last_error_ = backend_->last_error();
        backend_.reset();
        return false;
    }

    if (!create_shaders()) {
        shutdown();
        return false;
    }

    if (!create_gpu_objects()) {
        shutdown();
        return false;
    }

    const WindowSize window_size = window_.size();

    resize(window_size.width, window_size.height);

    initialized_ = true;

    return true;
}

void OpenGLRenderer3D::shutdown() {
    if (backend_) {
        backend_->make_current();
        destroy_gpu_objects();

        if (program_) {
            backend_->destroy_program(program_);
            program_ = 0;
        }

        if (vertex_shader_) {
            backend_->destroy_shader(vertex_shader_);
            vertex_shader_ = 0;
        }

        if (fragment_shader_) {
            backend_->destroy_shader(fragment_shader_);
            fragment_shader_ = 0;
        }

        backend_->shutdown();
        backend_.reset();
    }

    initialized_ = false;
    frame_active_ = false;
}

bool OpenGLRenderer3D::is_initialized() const noexcept {
    return initialized_;
}

const std::string& OpenGLRenderer3D::last_error() const noexcept {
    return last_error_;
}

const char* OpenGLRenderer3D::graphics_version() const noexcept {
    return backend_ ? backend_->version_string() : "";
}

const char* OpenGLRenderer3D::graphics_renderer() const noexcept {
    return backend_ ? backend_->renderer_string() : "";
}

bool OpenGLRenderer3D::create_shaders() {
    std::string log;

    if (!backend_->compile_shader(
        ShaderStage::Vertex,
        VERTEX_SHADER_SOURCE,
        vertex_shader_,
        log
    )) {
        last_error_ = "Vertex shader compilation failed: " + log;
        return false;
    }

    if (!backend_->compile_shader(
        ShaderStage::Fragment,
        FRAGMENT_SHADER_SOURCE,
        fragment_shader_,
        log
    )) {
        last_error_ = "Fragment shader compilation failed: " + log;
        return false;
    }

    if (!backend_->create_program(program_, log)) {
        last_error_ = "Program creation failed: " + log;
        return false;
    }

    if (!backend_->attach_shader(program_, vertex_shader_, log)) {
        last_error_ = "Vertex shader attachment failed: " + log;
        return false;
    }

    if (!backend_->attach_shader(program_, fragment_shader_, log)) {
        last_error_ = "Fragment shader attachment failed: " + log;
        return false;
    }

    if (!backend_->link_program(program_, log)) {
        last_error_ = "Shader program linking failed: " + log;
        return false;
    }

    return true;
}

bool OpenGLRenderer3D::create_gpu_objects() {
    vertex_array_ = backend_->create_vertex_array();

    if (!vertex_array_) {
        last_error_ = "Failed to create the GPU vertex array.";
        return false;
    }

    vertex_buffer_ = backend_->create_buffer();

    if (!vertex_buffer_) {
        last_error_ = "Failed to create the GPU vertex buffer.";
        return false;
    }

    index_buffer_ = backend_->create_buffer();

    if (!index_buffer_) {
        last_error_ = "Failed to create the GPU index buffer.";
        return false;
    }

    backend_->bind_vertex_array(vertex_array_);

    backend_->upload_array_buffer(
        vertex_buffer_,
        nullptr,
        0,
        true
    );

    backend_->upload_element_buffer(
        index_buffer_,
        nullptr,
        0,
        true
    );

    const std::size_t stride = sizeof(GPUVertex);

    backend_->configure_vertex_attribute(
        0,
        3,
        stride,
        offsetof(GPUVertex, position)
    );

    backend_->configure_vertex_attribute(
        1,
        3,
        stride,
        offsetof(GPUVertex, normal)
    );

    backend_->configure_vertex_attribute(
        2,
        2,
        stride,
        offsetof(GPUVertex, uv)
    );

    backend_->bind_vertex_array(0);

    return true;
}

void OpenGLRenderer3D::destroy_gpu_objects() {
    if (!backend_) {
        return;
    }

    if (index_buffer_) {
        backend_->destroy_buffer(index_buffer_);
        index_buffer_ = 0;
    }

    if (vertex_buffer_) {
        backend_->destroy_buffer(vertex_buffer_);
        vertex_buffer_ = 0;
    }

    if (vertex_array_) {
        backend_->destroy_vertex_array(vertex_array_);
        vertex_array_ = 0;
    }
}

void OpenGLRenderer3D::resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    width_ = width;
    height_ = height;

    if (backend_) {
        backend_->set_viewport(width_, height_);
    }
}

void OpenGLRenderer3D::add_light(const Light3D& light) {
    if (lights_.size() < 8) {
        lights_.push_back(light);
    }
}

void OpenGLRenderer3D::clear_lights() {
    lights_.clear();
}

void OpenGLRenderer3D::begin_frame(
    const Camera3D& camera,
    Color3 clear_color
) {
    if (!initialized_ || !backend_) {
        return;
    }

    camera_ = camera;

    const WindowSize current_size = window_.size();

    if (current_size.width > 0 &&
        current_size.height > 0 &&
        (current_size.width != width_ ||
         current_size.height != height_)) {
        resize(current_size.width, current_size.height);
    }

    backend_->make_current();

    backend_->clear(
        clear_color.r,
        clear_color.g,
        clear_color.b,
        1.0f
    );

    RenderState state;
    state.depth_test_enabled = true;
    state.depth_write_enabled = true;
    state.depth_function = DepthFunction::Less;
    state.face_culling_enabled = true;
    state.cull_mode = CullMode::Back;
    state.front_face = FrontFace::CounterClockwise;

    backend_->apply_render_state(state);

    frame_active_ = true;
}

void OpenGLRenderer3D::to_column_major(
    const Matrix4& source,
    float destination[16]
) {
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            destination[column * 4 + row] =
                source.m[row][column];
        }
    }
}

void OpenGLRenderer3D::upload_matrix(
    const char* uniform,
    const Matrix4& matrix
) {
    float values[16]{};

    to_column_major(matrix, values);

    backend_->uniform_mat4(
        backend_->uniform_location(program_, uniform),
        values
    );
}

void OpenGLRenderer3D::upload_lighting() {
    backend_->uniform_int(
        backend_->uniform_location(program_, "uLightCount"),
        static_cast<int>(lights_.size())
    );

    for (std::size_t i = 0; i < lights_.size(); ++i) {
        const Light3D& light = lights_[i];

        const std::string prefix =
            "uLights[" + std::to_string(i) + "].";

        backend_->uniform_int(
            backend_->uniform_location(
                program_,
                (prefix + "type").c_str()
            ),
            light.type == LightType::Directional ? 0 : 1
        );

        backend_->uniform_vec3(
            backend_->uniform_location(
                program_,
                (prefix + "position").c_str()
            ),
            light.position.x,
            light.position.y,
            light.position.z
        );

        backend_->uniform_vec3(
            backend_->uniform_location(
                program_,
                (prefix + "direction").c_str()
            ),
            light.direction.x,
            light.direction.y,
            light.direction.z
        );

        backend_->uniform_vec3(
            backend_->uniform_location(
                program_,
                (prefix + "color").c_str()
            ),
            light.color.r,
            light.color.g,
            light.color.b
        );

        backend_->uniform_float(
            backend_->uniform_location(
                program_,
                (prefix + "intensity").c_str()
            ),
            light.intensity
        );

        backend_->uniform_float(
            backend_->uniform_location(
                program_,
                (prefix + "range").c_str()
            ),
            light.range
        );
    }
}

void OpenGLRenderer3D::draw_mesh(
    const Mesh& mesh,
    const Transform3D& transform,
    const Material& material
) {
    if (!initialized_ || !frame_active_ ||
        mesh.empty() || !backend_) {
        return;
    }

    if (mesh.vertices.size() >
        static_cast<std::size_t>(0x7fffffff) ||
        mesh.indices.size() >
        static_cast<std::size_t>(0x7fffffff)) {
        return;
    }

    std::vector<GPUVertex> vertices;
    vertices.reserve(mesh.vertices.size());

    for (const Vertex3D& vertex : mesh.vertices) {
        GPUVertex gpu_vertex{};

        gpu_vertex.position[0] = vertex.position.x;
        gpu_vertex.position[1] = vertex.position.y;
        gpu_vertex.position[2] = vertex.position.z;

        gpu_vertex.normal[0] = vertex.normal.x;
        gpu_vertex.normal[1] = vertex.normal.y;
        gpu_vertex.normal[2] = vertex.normal.z;

        gpu_vertex.uv[0] = vertex.u;
        gpu_vertex.uv[1] = vertex.v;

        vertices.push_back(gpu_vertex);
    }

    const Matrix4 model_matrix = transform.matrix();
    const Matrix4 view_matrix = camera_.view_matrix();

    const float aspect =
        height_ > 0
            ? static_cast<float>(width_) /
              static_cast<float>(height_)
            : 1.0f;

    const Matrix4 projection_matrix =
        camera_.projection_matrix(aspect);

    backend_->use_program(program_);

    upload_matrix("uModel", model_matrix);
    upload_matrix("uView", view_matrix);
    upload_matrix("uProjection", projection_matrix);

    backend_->uniform_vec3(
        backend_->uniform_location(
            program_,
            "uCameraPosition"
        ),
        camera_.position.x,
        camera_.position.y,
        camera_.position.z
    );

    backend_->uniform_vec3(
        backend_->uniform_location(program_, "uDiffuse"),
        material.diffuse.r,
        material.diffuse.g,
        material.diffuse.b
    );

    backend_->uniform_vec3(
        backend_->uniform_location(program_, "uAmbient"),
        material.ambient.r,
        material.ambient.g,
        material.ambient.b
    );

    backend_->uniform_vec3(
        backend_->uniform_location(program_, "uSpecular"),
        material.specular.r,
        material.specular.g,
        material.specular.b
    );

    backend_->uniform_float(
        backend_->uniform_location(program_, "uShininess"),
        material.shininess
    );

    backend_->uniform_float(
        backend_->uniform_location(program_, "uOpacity"),
        material.opacity
    );

    upload_lighting();

    RenderState state;
    state.depth_test_enabled = true;
    state.depth_write_enabled = material.opacity >= 1.0f;
    state.face_culling_enabled = true;
    state.cull_mode = CullMode::Back;

    if (material.opacity < 1.0f) {
        state.blending.enabled = true;
        state.blending.source_color = BlendFactor::SourceAlpha;
        state.blending.destination_color =
            BlendFactor::OneMinusSourceAlpha;
        state.blending.source_alpha = BlendFactor::One;
        state.blending.destination_alpha =
            BlendFactor::OneMinusSourceAlpha;
    }

    backend_->apply_render_state(state);

    backend_->bind_vertex_array(vertex_array_);

    backend_->upload_array_buffer(
        vertex_buffer_,
        vertices.data(),
        vertices.size() * sizeof(GPUVertex),
        true
    );

    backend_->upload_element_buffer(
        index_buffer_,
        mesh.indices.data(),
        mesh.indices.size() * sizeof(std::uint32_t),
        true
    );

    backend_->draw_indexed(
        static_cast<std::uint32_t>(mesh.indices.size())
    );

    backend_->bind_vertex_array(0);
}

void OpenGLRenderer3D::draw_model(
    const Model& model,
    const Transform3D& transform
) {
    draw_mesh(model.mesh, transform, model.material);
}

void OpenGLRenderer3D::end_frame() {
    if (!initialized_ || !frame_active_ || !backend_) {
        return;
    }

    backend_->use_program(0);
    backend_->present();

    frame_active_ = false;
}

} // namespace sao

#endif
