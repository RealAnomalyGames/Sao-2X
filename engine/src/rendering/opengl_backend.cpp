
#include "sao/rendering/opengl_backend.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>
#include <vector>

namespace sao {

namespace {

using GLenum = unsigned int;
using GLuint = unsigned int;
using GLint = int;
using GLsizei = int;
using GLboolean = unsigned char;
using GLfloat = float;
using GLchar = char;
using GLbitfield = unsigned int;
using GLsizeiptr = std::ptrdiff_t;

constexpr GLenum GL_VERSION_VALUE = 0x1F02;
constexpr GLenum GL_RENDERER_VALUE = 0x1F01;

constexpr GLenum GL_VERTEX_SHADER_VALUE = 0x8B31;
constexpr GLenum GL_FRAGMENT_SHADER_VALUE = 0x8B30;
constexpr GLenum GL_GEOMETRY_SHADER_VALUE = 0x8DD9;

constexpr GLenum GL_COMPILE_STATUS_VALUE = 0x8B81;
constexpr GLenum GL_LINK_STATUS_VALUE = 0x8B82;
constexpr GLenum GL_INFO_LOG_LENGTH_VALUE = 0x8B84;

constexpr GLenum GL_DEPTH_TEST_VALUE = 0x0B71;
constexpr GLenum GL_BLEND_VALUE = 0x0BE2;
constexpr GLenum GL_CULL_FACE_VALUE = 0x0B44;

constexpr GLenum GL_FRONT_VALUE = 0x0404;
constexpr GLenum GL_BACK_VALUE = 0x0405;
constexpr GLenum GL_FRONT_AND_BACK_VALUE = 0x0408;

constexpr GLenum GL_CW_VALUE = 0x0900;
constexpr GLenum GL_CCW_VALUE = 0x0901;

constexpr GLenum GL_NEVER_VALUE = 0x0200;
constexpr GLenum GL_LESS_VALUE = 0x0201;
constexpr GLenum GL_EQUAL_VALUE = 0x0202;
constexpr GLenum GL_LEQUAL_VALUE = 0x0203;
constexpr GLenum GL_GREATER_VALUE = 0x0204;
constexpr GLenum GL_NOTEQUAL_VALUE = 0x0205;
constexpr GLenum GL_GEQUAL_VALUE = 0x0206;
constexpr GLenum GL_ALWAYS_VALUE = 0x0207;

constexpr GLenum GL_ZERO_VALUE = 0;
constexpr GLenum GL_ONE_VALUE = 1;
constexpr GLenum GL_SRC_COLOR_VALUE = 0x0300;
constexpr GLenum GL_ONE_MINUS_SRC_COLOR_VALUE = 0x0301;
constexpr GLenum GL_SRC_ALPHA_VALUE = 0x0302;
constexpr GLenum GL_ONE_MINUS_SRC_ALPHA_VALUE = 0x0303;
constexpr GLenum GL_DST_ALPHA_VALUE = 0x0304;
constexpr GLenum GL_ONE_MINUS_DST_ALPHA_VALUE = 0x0305;
constexpr GLenum GL_DST_COLOR_VALUE = 0x0306;
constexpr GLenum GL_ONE_MINUS_DST_COLOR_VALUE = 0x0307;

constexpr GLenum GL_FUNC_ADD_VALUE = 0x8006;
constexpr GLenum GL_FUNC_SUBTRACT_VALUE = 0x800A;
constexpr GLenum GL_FUNC_REVERSE_SUBTRACT_VALUE = 0x800B;
constexpr GLenum GL_MIN_VALUE = 0x8007;
constexpr GLenum GL_MAX_VALUE = 0x8008;

constexpr GLenum GL_POINT_VALUE = 0x1B00;
constexpr GLenum GL_LINE_VALUE = 0x1B01;
constexpr GLenum GL_FILL_VALUE = 0x1B02;

constexpr GLenum GL_TEXTURE0_VALUE = 0x84C0;

constexpr GLenum GL_TEXTURE_MIN_FILTER_VALUE = 0x2801;
constexpr GLenum GL_TEXTURE_MAG_FILTER_VALUE = 0x2800;
constexpr GLenum GL_TEXTURE_WRAP_S_VALUE = 0x2802;
constexpr GLenum GL_TEXTURE_WRAP_T_VALUE = 0x2803;

constexpr GLenum GL_NEAREST_VALUE = 0x2600;
constexpr GLenum GL_LINEAR_VALUE = 0x2601;
constexpr GLenum GL_NEAREST_MIPMAP_NEAREST_VALUE = 0x2700;
constexpr GLenum GL_LINEAR_MIPMAP_NEAREST_VALUE = 0x2701;
constexpr GLenum GL_NEAREST_MIPMAP_LINEAR_VALUE = 0x2702;
constexpr GLenum GL_LINEAR_MIPMAP_LINEAR_VALUE = 0x2703;

constexpr GLenum GL_REPEAT_VALUE = 0x2901;
constexpr GLenum GL_MIRRORED_REPEAT_VALUE = 0x8370;
constexpr GLenum GL_CLAMP_TO_EDGE_VALUE = 0x812F;
constexpr GLenum GL_CLAMP_TO_BORDER_VALUE = 0x812D;

constexpr GLenum GL_TEXTURE_MAX_ANISOTROPY_EXT_VALUE = 0x84FE;

constexpr int WGL_CONTEXT_MAJOR_VERSION_VALUE = 0x2091;
constexpr int WGL_CONTEXT_MINOR_VERSION_VALUE = 0x2092;
constexpr int WGL_CONTEXT_PROFILE_MASK_VALUE = 0x9126;
constexpr int WGL_CONTEXT_CORE_PROFILE_BIT_VALUE = 0x00000001;

using CreateContextAttribsARB = HGLRC(WINAPI*)(
    HDC,
    HGLRC,
    const int*
);

bool valid_wgl_proc(PROC proc) {
    return proc &&
           proc != reinterpret_cast<PROC>(1) &&
           proc != reinterpret_cast<PROC>(2) &&
           proc != reinterpret_cast<PROC>(3) &&
           proc != reinterpret_cast<PROC>(-1);
}

PROC load_gl_proc(const char* name) {
    PROC proc = wglGetProcAddress(name);

    if (valid_wgl_proc(proc)) {
        return proc;
    }

    HMODULE module = GetModuleHandleW(L"opengl32.dll");

    if (!module) {
        return nullptr;
    }

    return reinterpret_cast<PROC>(
        GetProcAddress(module, name)
    );
}

GLenum to_gl_depth_function(DepthFunction function) {
    switch (function) {
        case DepthFunction::Never: return GL_NEVER_VALUE;
        case DepthFunction::Less: return GL_LESS_VALUE;
        case DepthFunction::LessEqual: return GL_LEQUAL_VALUE;
        case DepthFunction::Equal: return GL_EQUAL_VALUE;
        case DepthFunction::GreaterEqual: return GL_GEQUAL_VALUE;
        case DepthFunction::Greater: return GL_GREATER_VALUE;
        case DepthFunction::NotEqual: return GL_NOTEQUAL_VALUE;
        case DepthFunction::Always: return GL_ALWAYS_VALUE;
    }

    return GL_LESS_VALUE;
}

GLenum to_gl_blend_factor(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::Zero: return GL_ZERO_VALUE;
        case BlendFactor::One: return GL_ONE_VALUE;
        case BlendFactor::SourceColor: return GL_SRC_COLOR_VALUE;
        case BlendFactor::OneMinusSourceColor:
            return GL_ONE_MINUS_SRC_COLOR_VALUE;
        case BlendFactor::DestinationColor:
            return GL_DST_COLOR_VALUE;
        case BlendFactor::OneMinusDestinationColor:
            return GL_ONE_MINUS_DST_COLOR_VALUE;
        case BlendFactor::SourceAlpha:
            return GL_SRC_ALPHA_VALUE;
        case BlendFactor::OneMinusSourceAlpha:
            return GL_ONE_MINUS_SRC_ALPHA_VALUE;
        case BlendFactor::DestinationAlpha:
            return GL_DST_ALPHA_VALUE;
        case BlendFactor::OneMinusDestinationAlpha:
            return GL_ONE_MINUS_DST_ALPHA_VALUE;
    }

    return GL_ONE_VALUE;
}

GLenum to_gl_blend_operation(BlendOperation operation) {
    switch (operation) {
        case BlendOperation::Add: return GL_FUNC_ADD_VALUE;
        case BlendOperation::Subtract:
            return GL_FUNC_SUBTRACT_VALUE;
        case BlendOperation::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT_VALUE;
        case BlendOperation::Minimum: return GL_MIN_VALUE;
        case BlendOperation::Maximum: return GL_MAX_VALUE;
    }

    return GL_FUNC_ADD_VALUE;
}

GLenum to_gl_texture_filter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST_VALUE;
        case TextureFilter::Linear: return GL_LINEAR_VALUE;
        case TextureFilter::NearestMipmapNearest:
            return GL_NEAREST_MIPMAP_NEAREST_VALUE;
        case TextureFilter::LinearMipmapNearest:
            return GL_LINEAR_MIPMAP_NEAREST_VALUE;
        case TextureFilter::NearestMipmapLinear:
            return GL_NEAREST_MIPMAP_LINEAR_VALUE;
        case TextureFilter::LinearMipmapLinear:
            return GL_LINEAR_MIPMAP_LINEAR_VALUE;
    }

    return GL_LINEAR_VALUE;
}

GLenum to_gl_texture_wrap(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT_VALUE;
        case TextureWrap::MirroredRepeat:
            return GL_MIRRORED_REPEAT_VALUE;
        case TextureWrap::ClampToEdge:
            return GL_CLAMP_TO_EDGE_VALUE;
        case TextureWrap::ClampToBorder:
            return GL_CLAMP_TO_BORDER_VALUE;
    }

    return GL_REPEAT_VALUE;
}

} // namespace

struct OpenGLBackend::GLFunctions {
#define SAO_GL_FUNCTION(return_type, name, arguments) \
    using name##Type = return_type(APIENTRY*) arguments; \
    name##Type name = nullptr

    SAO_GL_FUNCTION(const unsigned char*, GetString, (GLenum));
    SAO_GL_FUNCTION(GLuint, CreateShader, (GLenum));
    SAO_GL_FUNCTION(void, ShaderSource,
        (GLuint, GLsizei, const GLchar* const*, const GLint*));
    SAO_GL_FUNCTION(void, CompileShader, (GLuint));
    SAO_GL_FUNCTION(void, GetShaderiv, (GLuint, GLenum, GLint*));
    SAO_GL_FUNCTION(void, GetShaderInfoLog,
        (GLuint, GLsizei, GLsizei*, GLchar*));
    SAO_GL_FUNCTION(void, DeleteShader, (GLuint));

    SAO_GL_FUNCTION(GLuint, CreateProgram, ());
    SAO_GL_FUNCTION(void, AttachShader, (GLuint, GLuint));
    SAO_GL_FUNCTION(void, LinkProgram, (GLuint));
    SAO_GL_FUNCTION(void, GetProgramiv, (GLuint, GLenum, GLint*));
    SAO_GL_FUNCTION(void, GetProgramInfoLog,
        (GLuint, GLsizei, GLsizei*, GLchar*));
    SAO_GL_FUNCTION(void, DeleteProgram, (GLuint));
    SAO_GL_FUNCTION(void, UseProgram, (GLuint));

    SAO_GL_FUNCTION(void, Enable, (GLenum));
    SAO_GL_FUNCTION(void, Disable, (GLenum));
    SAO_GL_FUNCTION(void, DepthMask, (GLboolean));
    SAO_GL_FUNCTION(void, DepthFunc, (GLenum));
    SAO_GL_FUNCTION(void, CullFace, (GLenum));
    SAO_GL_FUNCTION(void, FrontFace, (GLenum));
    SAO_GL_FUNCTION(void, BlendFuncSeparate,
        (GLenum, GLenum, GLenum, GLenum));
    SAO_GL_FUNCTION(void, BlendEquationSeparate,
        (GLenum, GLenum));
    SAO_GL_FUNCTION(void, PolygonMode, (GLenum, GLenum));

    SAO_GL_FUNCTION(void, ActiveTexture, (GLenum));
    SAO_GL_FUNCTION(void, GenSamplers, (GLsizei, GLuint*));
    SAO_GL_FUNCTION(void, DeleteSamplers, (GLsizei, const GLuint*));
    SAO_GL_FUNCTION(void, SamplerParameteri,
        (GLuint, GLenum, GLint));
    SAO_GL_FUNCTION(void, SamplerParameterf,
        (GLuint, GLenum, GLfloat));
    SAO_GL_FUNCTION(void, BindSampler, (GLuint, GLuint));

#undef SAO_GL_FUNCTION
};

OpenGLBackend::OpenGLBackend(Win32Window& window)
    : window_(window) {
}

OpenGLBackend::~OpenGLBackend() {
    shutdown();
}

const char* OpenGLBackend::name() const noexcept {
    return "OpenGL 3.3 Core (WGL)";
}

bool OpenGLBackend::is_available() const noexcept {
    return initialized_ && rendering_context_ != nullptr;
}

const std::string& OpenGLBackend::renderer_name() const noexcept {
    return renderer_name_;
}

const std::string& OpenGLBackend::version_string() const noexcept {
    return version_string_;
}

bool OpenGLBackend::create_context(std::string& error) {
    HWND hwnd = window_.native_handle();

    if (!hwnd) {
        error = "The Win32 window has no native handle.";
        return false;
    }

    device_context_ = GetDC(hwnd);

    if (!device_context_) {
        error = "Could not acquire the window device context.";
        return false;
    }

    PIXELFORMATDESCRIPTOR pixel_format{};
    pixel_format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format.nVersion = 1;
    pixel_format.dwFlags =
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER;
    pixel_format.iPixelType = PFD_TYPE_RGBA;
    pixel_format.cColorBits = 32;
    pixel_format.cAlphaBits = 8;
    pixel_format.cDepthBits = 24;
    pixel_format.cStencilBits = 8;
    pixel_format.iLayerType = PFD_MAIN_PLANE;

    const int format = ChoosePixelFormat(
        device_context_,
        &pixel_format
    );

    if (format == 0) {
        error = "Could not choose a Windows OpenGL pixel format.";
        return false;
    }

    if (!SetPixelFormat(device_context_, format, &pixel_format)) {
        error = "Could not set the Windows OpenGL pixel format.";
        return false;
    }

    HGLRC temporary_context = wglCreateContext(device_context_);

    if (!temporary_context) {
        error = "Could not create the temporary WGL context.";
        return false;
    }

    if (!wglMakeCurrent(device_context_, temporary_context)) {
        wglDeleteContext(temporary_context);
        error = "Could not activate the temporary WGL context.";
        return false;
    }

    auto create_context = reinterpret_cast<CreateContextAttribsARB>(
        wglGetProcAddress("wglCreateContextAttribsARB")
    );

    if (!create_context) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(temporary_context);
        error = "WGL_ARB_create_context is unavailable.";
        return false;
    }

    const int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_VALUE, 3,
        WGL_CONTEXT_MINOR_VERSION_VALUE, 3,
        WGL_CONTEXT_PROFILE_MASK_VALUE,
        WGL_CONTEXT_CORE_PROFILE_BIT_VALUE,
        0
    };

    HGLRC modern_context = create_context(
        device_context_,
        nullptr,
        attributes
    );

    if (!modern_context) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(temporary_context);
        error = "The graphics driver could not create an OpenGL 3.3 Core context.";
        return false;
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(temporary_context);

    rendering_context_ = modern_context;

    if (!wglMakeCurrent(device_context_, rendering_context_)) {
        error = "Could not activate the OpenGL 3.3 context.";
        return false;
    }

    return true;
}

bool OpenGLBackend::load_functions(std::string& error) {
    gl_ = std::make_unique<GLFunctions>();

#define SAO_LOAD_GL(name) \
    gl_->name = reinterpret_cast<GLFunctions::name##Type>( \
        load_gl_proc("gl" #name) \
    ); \
    if (!gl_->name) { \
        error = "Missing OpenGL function: gl" #name; \
        return false; \
    }

    SAO_LOAD_GL(GetString)
    SAO_LOAD_GL(CreateShader)
    SAO_LOAD_GL(ShaderSource)
    SAO_LOAD_GL(CompileShader)
    SAO_LOAD_GL(GetShaderiv)
    SAO_LOAD_GL(GetShaderInfoLog)
    SAO_LOAD_GL(DeleteShader)

    SAO_LOAD_GL(CreateProgram)
    SAO_LOAD_GL(AttachShader)
    SAO_LOAD_GL(LinkProgram)
    SAO_LOAD_GL(GetProgramiv)
    SAO_LOAD_GL(GetProgramInfoLog)
    SAO_LOAD_GL(DeleteProgram)
    SAO_LOAD_GL(UseProgram)

    SAO_LOAD_GL(Enable)
    SAO_LOAD_GL(Disable)
    SAO_LOAD_GL(DepthMask)
    SAO_LOAD_GL(DepthFunc)
    SAO_LOAD_GL(CullFace)
    SAO_LOAD_GL(FrontFace)
    SAO_LOAD_GL(BlendFuncSeparate)
    SAO_LOAD_GL(BlendEquationSeparate)
    SAO_LOAD_GL(PolygonMode)

    SAO_LOAD_GL(ActiveTexture)
    SAO_LOAD_GL(GenSamplers)
    SAO_LOAD_GL(DeleteSamplers)
    SAO_LOAD_GL(SamplerParameteri)
    SAO_LOAD_GL(SamplerParameterf)
    SAO_LOAD_GL(BindSampler)

#undef SAO_LOAD_GL

    const auto* renderer = gl_->GetString(GL_RENDERER_VALUE);
    const auto* version = gl_->GetString(GL_VERSION_VALUE);

    renderer_name_ = renderer
        ? reinterpret_cast<const char*>(renderer)
        : "Unknown OpenGL renderer";

    version_string_ = version
        ? reinterpret_cast<const char*>(version)
        : "Unknown OpenGL version";

    return true;
}

bool OpenGLBackend::initialize() {
    if (initialized_) {
        return true;
    }

    std::string error;

    if (!create_context(error)) {
        shutdown();
        return false;
    }

    if (!load_functions(error)) {
        shutdown();
        return false;
    }

    initialized_ = true;
    return true;
}

void OpenGLBackend::destroy_context() noexcept {
    if (wglGetCurrentContext() == rendering_context_) {
        wglMakeCurrent(nullptr, nullptr);
    }

    if (rendering_context_) {
        wglDeleteContext(rendering_context_);
        rendering_context_ = nullptr;
    }

    if (device_context_) {
        if (window_.native_handle()) {
            ReleaseDC(window_.native_handle(), device_context_);
        }

        device_context_ = nullptr;
    }
}

void OpenGLBackend::shutdown() {
    if (rendering_context_ && device_context_) {
        wglMakeCurrent(device_context_, rendering_context_);

        if (gl_) {
            for (const auto& entry : sampler_objects_) {
                const GLuint sampler = entry.second;
                gl_->DeleteSamplers(1, &sampler);
            }
        }
    }

    sampler_objects_.clear();

    destroy_context();

    gl_.reset();

    renderer_name_.clear();
    version_string_.clear();

    initialized_ = false;
}

bool OpenGLBackend::compile_shader(
    ShaderStage stage,
    const std::string& source,
    std::uint32_t& shader_handle,
    std::string& log
) {
    shader_handle = 0;

    if (!initialized_ || !gl_) {
        log = "OpenGL backend is not initialized.";
        return false;
    }

    GLenum gl_stage = 0;

    switch (stage) {
        case ShaderStage::Vertex:
            gl_stage = GL_VERTEX_SHADER_VALUE;
            break;

        case ShaderStage::Fragment:
            gl_stage = GL_FRAGMENT_SHADER_VALUE;
            break;

        case ShaderStage::Geometry:
            gl_stage = GL_GEOMETRY_SHADER_VALUE;
            break;

        case ShaderStage::Compute:
            log = "Compute shaders require OpenGL 4.3 or newer.";
            return false;
    }

    const GLuint shader = gl_->CreateShader(gl_stage);

    if (shader == 0) {
        log = "OpenGL could not create the shader object.";
        return false;
    }

    const GLchar* source_pointer = source.c_str();
    const GLint source_length = static_cast<GLint>(source.size());

    gl_->ShaderSource(
        shader,
        1,
        &source_pointer,
        &source_length
    );

    gl_->CompileShader(shader);

    GLint success = 0;
    gl_->GetShaderiv(shader, GL_COMPILE_STATUS_VALUE, &success);

    GLint log_length = 0;
    gl_->GetShaderiv(shader, GL_INFO_LOG_LENGTH_VALUE, &log_length);

    if (log_length > 1) {
        std::vector<GLchar> message(
            static_cast<std::size_t>(log_length),
            '\0'
        );

        GLsizei written = 0;

        gl_->GetShaderInfoLog(
            shader,
            log_length,
            &written,
            message.data()
        );

        log.assign(
            message.data(),
            static_cast<std::size_t>(std::max<GLsizei>(0, written))
        );
    }

    if (!success) {
        gl_->DeleteShader(shader);

        if (log.empty()) {
            log = "OpenGL shader compilation failed.";
        }

        return false;
    }

    shader_handle = shader;
    return true;
}

void OpenGLBackend::destroy_shader(std::uint32_t handle) {
    if (initialized_ && gl_ && handle != 0) {
        gl_->DeleteShader(handle);
    }
}

bool OpenGLBackend::create_program(
    std::uint32_t& program_handle,
    std::string& log
) {
    program_handle = 0;

    if (!initialized_ || !gl_) {
        log = "OpenGL backend is not initialized.";
        return false;
    }

    const GLuint program = gl_->CreateProgram();

    if (program == 0) {
        log = "OpenGL could not create a shader program.";
        return false;
    }

    program_handle = program;
    return true;
}

bool OpenGLBackend::attach_shader(
    std::uint32_t program_handle,
    std::uint32_t shader_handle,
    std::string& log
) {
    if (!initialized_ || !gl_ ||
        program_handle == 0 || shader_handle == 0) {
        log = "Invalid OpenGL program or shader handle.";
        return false;
    }

    gl_->AttachShader(program_handle, shader_handle);
    return true;
}

bool OpenGLBackend::link_program(
    std::uint32_t program_handle,
    std::string& log
) {
    if (!initialized_ || !gl_ || program_handle == 0) {
        log = "Invalid OpenGL program handle.";
        return false;
    }

    gl_->LinkProgram(program_handle);

    GLint success = 0;
    gl_->GetProgramiv(
        program_handle,
        GL_LINK_STATUS_VALUE,
        &success
    );

    GLint log_length = 0;
    gl_->GetProgramiv(
        program_handle,
        GL_INFO_LOG_LENGTH_VALUE,
        &log_length
    );

    if (log_length > 1) {
        std::vector<GLchar> message(
            static_cast<std::size_t>(log_length),
            '\0'
        );

        GLsizei written = 0;

        gl_->GetProgramInfoLog(
            program_handle,
            log_length,
            &written,
            message.data()
        );

        log.assign(
            message.data(),
            static_cast<std::size_t>(std::max<GLsizei>(0, written))
        );
    }

    if (!success) {
        if (log.empty()) {
            log = "OpenGL shader program linking failed.";
        }

        return false;
    }

    return true;
}

void OpenGLBackend::destroy_program(std::uint32_t handle) {
    if (initialized_ && gl_ && handle != 0) {
        gl_->DeleteProgram(handle);
    }
}

void OpenGLBackend::use_program(std::uint32_t handle) {
    if (initialized_ && gl_) {
        gl_->UseProgram(handle);
    }
}

void OpenGLBackend::apply_render_state(
    const RenderState& state
) {
    if (!initialized_ || !gl_) {
        return;
    }

    if (state.depth_test_enabled) {
        gl_->Enable(GL_DEPTH_TEST_VALUE);
    } else {
        gl_->Disable(GL_DEPTH_TEST_VALUE);
    }

    gl_->DepthMask(state.depth_write_enabled ? 1 : 0);
    gl_->DepthFunc(to_gl_depth_function(state.depth_function));

    if (state.face_culling_enabled &&
        state.cull_mode != CullMode::None) {
        gl_->Enable(GL_CULL_FACE_VALUE);

        GLenum mode = GL_BACK_VALUE;

        if (state.cull_mode == CullMode::Front) {
            mode = GL_FRONT_VALUE;
        } else if (state.cull_mode == CullMode::FrontAndBack) {
            mode = GL_FRONT_AND_BACK_VALUE;
        }

        gl_->CullFace(mode);
    } else {
        gl_->Disable(GL_CULL_FACE_VALUE);
    }

    gl_->FrontFace(
        state.front_face == FrontFace::Clockwise
            ? GL_CW_VALUE
            : GL_CCW_VALUE
    );

    if (state.blending.enabled) {
        gl_->Enable(GL_BLEND_VALUE);

        gl_->BlendFuncSeparate(
            to_gl_blend_factor(state.blending.source_color),
            to_gl_blend_factor(state.blending.destination_color),
            to_gl_blend_factor(state.blending.source_alpha),
            to_gl_blend_factor(state.blending.destination_alpha)
        );

        gl_->BlendEquationSeparate(
            to_gl_blend_operation(state.blending.color_operation),
            to_gl_blend_operation(state.blending.alpha_operation)
        );
    } else {
        gl_->Disable(GL_BLEND_VALUE);
    }

    gl_->PolygonMode(
        GL_FRONT_AND_BACK_VALUE,
        state.wireframe ? GL_LINE_VALUE : GL_FILL_VALUE
    );
}

void OpenGLBackend::apply_texture_sampler(
    unsigned int texture_unit,
    const TextureSampler& sampler
) {
    if (!initialized_ || !gl_) {
        return;
    }

    GLuint sampler_object = 0;

    auto existing = sampler_objects_.find(texture_unit);

    if (existing == sampler_objects_.end()) {
        gl_->GenSamplers(1, &sampler_object);

        if (sampler_object == 0) {
            return;
        }

        sampler_objects_[texture_unit] = sampler_object;
    } else {
        sampler_object = existing->second;
    }

    gl_->SamplerParameteri(
        sampler_object,
        GL_TEXTURE_MIN_FILTER_VALUE,
        static_cast<GLint>(to_gl_texture_filter(sampler.min_filter))
    );

    gl_->SamplerParameteri(
        sampler_object,
        GL_TEXTURE_MAG_FILTER_VALUE,
        static_cast<GLint>(to_gl_texture_filter(sampler.mag_filter))
    );

    gl_->SamplerParameteri(
        sampler_object,
        GL_TEXTURE_WRAP_S_VALUE,
        static_cast<GLint>(to_gl_texture_wrap(sampler.wrap_u))
    );

    gl_->SamplerParameteri(
        sampler_object,
        GL_TEXTURE_WRAP_T_VALUE,
        static_cast<GLint>(to_gl_texture_wrap(sampler.wrap_v))
    );

    if (sampler.anisotropy > 1.0f) {
        gl_->SamplerParameterf(
            sampler_object,
            GL_TEXTURE_MAX_ANISOTROPY_EXT_VALUE,
            sampler.anisotropy
        );
    }

    gl_->BindSampler(texture_unit, sampler_object);
}

} // namespace sao
