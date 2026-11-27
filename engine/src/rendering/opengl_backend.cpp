#ifdef _WIN32

#include "sao/rendering/opengl_backend.hpp"

#include <windows.h>
#include <GL/gl.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif

#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif

#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif

#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif

#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

#ifndef GL_DEPTH_TEST
#define GL_DEPTH_TEST 0x0B71
#endif

#ifndef GL_CULL_FACE
#define GL_CULL_FACE 0x0B44
#endif

#ifndef GL_BLEND
#define GL_BLEND 0x0BE2
#endif

#ifndef GL_SRC_ALPHA
#define GL_SRC_ALPHA 0x0302
#endif

#ifndef GL_ONE_MINUS_SRC_ALPHA
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#endif

#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD 0x8006
#endif

#ifndef GL_FUNC_SUBTRACT
#define GL_FUNC_SUBTRACT 0x800A
#endif

#ifndef GL_FUNC_REVERSE_SUBTRACT
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#endif

#ifndef GL_MIN
#define GL_MIN 0x8007
#endif

#ifndef GL_MAX
#define GL_MAX 0x8008
#endif

#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK 0x0408
#endif

#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif

#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif

#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif

#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif

#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif

#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D 0x0DE1
#endif

#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif

#ifndef GL_MIRRORED_REPEAT
#define GL_MIRRORED_REPEAT 0x8370
#endif

#ifndef GL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_MIN_FILTER 0x2801
#endif

#ifndef GL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MAG_FILTER 0x2800
#endif

#ifndef GL_TEXTURE_WRAP_S
#define GL_TEXTURE_WRAP_S 0x2802
#endif

#ifndef GL_TEXTURE_WRAP_T
#define GL_TEXTURE_WRAP_T 0x2803
#endif

#ifndef GL_TEXTURE_MIN_LOD
#define GL_TEXTURE_MIN_LOD 0x813A
#endif

#ifndef GL_TEXTURE_MAX_LOD
#define GL_TEXTURE_MAX_LOD 0x813B
#endif

#ifndef GL_UNSIGNED_INT
#define GL_UNSIGNED_INT 0x1405
#endif

#ifndef GL_DEPTH_COMPONENT
#define GL_DEPTH_COMPONENT 0x1902
#endif

#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#endif

#ifndef WGL_CONTEXT_MINOR_VERSION_ARB
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#endif

#ifndef WGL_CONTEXT_PROFILE_MASK_ARB
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#endif

#ifndef WGL_CONTEXT_CORE_PROFILE_BIT_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#endif

namespace sao {

namespace {

using GLenum = unsigned int;
using GLuint = unsigned int;
using GLint = int;
using GLsizei = int;
using GLboolean = unsigned char;
using GLchar = char;
using GLfloat = float;
using GLsizeiptr = std::ptrdiff_t;

using WglCreateContextAttribsARB =
    HGLRC(WINAPI*)(HDC, HGLRC, const int*);

using PFNGLCREATESHADERPROC =
    GLuint(APIENTRY*)(GLenum);

using PFNGLSHADERSOURCEPROC =
    void(APIENTRY*)(GLuint, GLsizei, const GLchar* const*, const GLint*);

using PFNGLCOMPILESHADERPROC =
    void(APIENTRY*)(GLuint);

using PFNGLGETSHADERIVPROC =
    void(APIENTRY*)(GLuint, GLenum, GLint*);

using PFNGLGETSHADERINFOLOGPROC =
    void(APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);

using PFNGLDELETESHADERPROC =
    void(APIENTRY*)(GLuint);

using PFNGLCREATEPROGRAMPROC =
    GLuint(APIENTRY*)();

using PFNGLATTACHSHADERPROC =
    void(APIENTRY*)(GLuint, GLuint);

using PFNGLLINKPROGRAMPROC =
    void(APIENTRY*)(GLuint);

using PFNGLGETPROGRAMIVPROC =
    void(APIENTRY*)(GLuint, GLenum, GLint*);

using PFNGLGETPROGRAMINFOLOGPROC =
    void(APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);

using PFNGLDELETEPROGRAMPROC =
    void(APIENTRY*)(GLuint);

using PFNGLUSEPROGRAMPROC =
    void(APIENTRY*)(GLuint);

using PFNGLGENBUFFERSPROC =
    void(APIENTRY*)(GLsizei, GLuint*);

using PFNGLDELETEBUFFERSPROC =
    void(APIENTRY*)(GLsizei, const GLuint*);

using PFNGLBINDBUFFERPROC =
    void(APIENTRY*)(GLenum, GLuint);

using PFNGLBUFFERDATAPROC =
    void(APIENTRY*)(GLenum, GLsizeiptr, const void*, GLenum);

using PFNGLGENVERTEXARRAYSPROC =
    void(APIENTRY*)(GLsizei, GLuint*);

using PFNGLDELETEVERTEXARRAYSPROC =
    void(APIENTRY*)(GLsizei, const GLuint*);

using PFNGLBINDVERTEXARRAYPROC =
    void(APIENTRY*)(GLuint);

using PFNGLENABLEVERTEXATTRIBARRAYPROC =
    void(APIENTRY*)(GLuint);

using PFNGLVERTEXATTRIBPOINTERPROC =
    void(APIENTRY*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);

using PFNGLDRAWELEMENTSPROC =
    void(APIENTRY*)(GLenum, GLsizei, GLenum, const void*);

using PFNGLGENTEXTURESPROC =
    void(APIENTRY*)(GLsizei, GLuint*);

using PFNGLDELETETEXTURESPROC =
    void(APIENTRY*)(GLsizei, const GLuint*);

using PFNGLBINDTEXTUREPROC =
    void(APIENTRY*)(GLenum, GLuint);

using PFNGLTEXIMAGE2DPROC =
    void(APIENTRY*)(GLenum, GLint, GLint, GLsizei, GLsizei,
                    GLint, GLenum, GLenum, const void*);

using PFNGLTEXPARAMETERIPROC =
    void(APIENTRY*)(GLenum, GLenum, GLint);

using PFNGLACTIVETEXTUREPROC =
    void(APIENTRY*)(GLenum);

using PFNGLPIXELSTOREIPROC =
    void(APIENTRY*)(GLenum, GLint);

using PFNGLGENSAMPLERSPROC =
    void(APIENTRY*)(GLsizei, GLuint*);

using PFNGLDELETESAMPLERSPROC =
    void(APIENTRY*)(GLsizei, const GLuint*);

using PFNGLSAMPLERPARAMETERIPROC =
    void(APIENTRY*)(GLuint, GLenum, GLint);

using PFNGLBINDSAMPLERPROC =
    void(APIENTRY*)(GLuint, GLuint);

using PFNGLBLENDFUNCSEPARATEPROC =
    void(APIENTRY*)(GLenum, GLenum, GLenum, GLenum);

using PFNGLBLENDEQUATIONSEPARATEPROC =
    void(APIENTRY*)(GLenum, GLenum);

using PFNGLDEPTHFUNCPROC =
    void(APIENTRY*)(GLenum);

using PFNGLDEPTHMASKPROC =
    void(APIENTRY*)(GLboolean);

using PFNGLCULLFACEPROC =
    void(APIENTRY*)(GLenum);

using PFNGLFRONTFACEPROC =
    void(APIENTRY*)(GLenum);

using PFNGLPOLYGONMODEPROC =
    void(APIENTRY*)(GLenum, GLenum);

using PFNGLGETUNIFORMLOCATIONPROC =
    GLint(APIENTRY*)(GLuint, const GLchar*);

using PFNGLUNIFORM1IPROC =
    void(APIENTRY*)(GLint, GLint);

using PFNGLUNIFORM1FPROC =
    void(APIENTRY*)(GLint, GLfloat);

using PFNGLUNIFORM3FPROC =
    void(APIENTRY*)(GLint, GLfloat, GLfloat, GLfloat);

using PFNGLUNIFORMMATRIX4FVPROC =
    void(APIENTRY*)(GLint, GLsizei, GLboolean, const GLfloat*);

using PFNGLVIEWPORTPROC =
    void(APIENTRY*)(GLint, GLint, GLsizei, GLsizei);

using PFNGLCLEARCOLORPROC =
    void(APIENTRY*)(GLfloat, GLfloat, GLfloat, GLfloat);

using PFNGLCLEARPROC =
    void(APIENTRY*)(unsigned int);

GLenum shader_stage_to_gl(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:
            return GL_VERTEX_SHADER;

        case ShaderStage::Fragment:
            return GL_FRAGMENT_SHADER;

        case ShaderStage::Geometry:
            return GL_GEOMETRY_SHADER;

        case ShaderStage::Compute:
            return 0;
    }

    return 0;
}

GLenum blend_factor_to_gl(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::Zero: return GL_ZERO;
        case BlendFactor::One: return GL_ONE;
        case BlendFactor::SourceColor: return GL_SRC_COLOR;
        case BlendFactor::OneMinusSourceColor:
            return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DestinationColor: return GL_DST_COLOR;
        case BlendFactor::OneMinusDestinationColor:
            return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SourceAlpha: return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSourceAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DestinationAlpha: return GL_DST_ALPHA;
        case BlendFactor::OneMinusDestinationAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
    }

    return GL_ONE;
}

GLenum blend_operation_to_gl(BlendOperation operation) {
    switch (operation) {
        case BlendOperation::Add: return GL_FUNC_ADD;
        case BlendOperation::Subtract: return GL_FUNC_SUBTRACT;
        case BlendOperation::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT;
        case BlendOperation::Minimum: return GL_MIN;
        case BlendOperation::Maximum: return GL_MAX;
    }

    return GL_FUNC_ADD;
}

GLenum depth_function_to_gl(DepthFunction function) {
    switch (function) {
        case DepthFunction::Never: return GL_NEVER;
        case DepthFunction::Less: return GL_LESS;
        case DepthFunction::LessEqual: return GL_LEQUAL;
        case DepthFunction::Equal: return GL_EQUAL;
        case DepthFunction::GreaterEqual: return GL_GEQUAL;
        case DepthFunction::Greater: return GL_GREATER;
        case DepthFunction::NotEqual: return GL_NOTEQUAL;
        case DepthFunction::Always: return GL_ALWAYS;
    }

    return GL_LESS;
}

GLenum texture_filter_to_gl(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest:
            return GL_NEAREST;

        case TextureFilter::Linear:
            return GL_LINEAR;

        case TextureFilter::NearestMipmapNearest:
            return GL_NEAREST_MIPMAP_NEAREST;

        case TextureFilter::LinearMipmapNearest:
            return GL_LINEAR_MIPMAP_NEAREST;

        case TextureFilter::NearestMipmapLinear:
            return GL_NEAREST_MIPMAP_LINEAR;

        case TextureFilter::LinearMipmapLinear:
            return GL_LINEAR_MIPMAP_LINEAR;
    }

    return GL_LINEAR;
}

GLenum texture_wrap_to_gl(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:
            return GL_REPEAT;

        case TextureWrap::MirroredRepeat:
            return GL_MIRRORED_REPEAT;

        case TextureWrap::ClampToEdge:
            return GL_CLAMP_TO_EDGE;

        case TextureWrap::ClampToBorder:
            return GL_CLAMP_TO_BORDER;
    }

    return GL_REPEAT;
}

} // namespace

struct OpenGLBackend::Functions {
    PFNGLCREATESHADERPROC CreateShader = nullptr;
    PFNGLSHADERSOURCEPROC ShaderSource = nullptr;
    PFNGLCOMPILESHADERPROC CompileShader = nullptr;
    PFNGLGETSHADERIVPROC GetShaderiv = nullptr;
    PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog = nullptr;
    PFNGLDELETESHADERPROC DeleteShader = nullptr;

    PFNGLCREATEPROGRAMPROC CreateProgram = nullptr;
    PFNGLATTACHSHADERPROC AttachShader = nullptr;
    PFNGLLINKPROGRAMPROC LinkProgram = nullptr;
    PFNGLGETPROGRAMIVPROC GetProgramiv = nullptr;
    PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog = nullptr;
    PFNGLDELETEPROGRAMPROC DeleteProgram = nullptr;
    PFNGLUSEPROGRAMPROC UseProgram = nullptr;

    PFNGLGENBUFFERSPROC GenBuffers = nullptr;
    PFNGLDELETEBUFFERSPROC DeleteBuffers = nullptr;
    PFNGLBINDBUFFERPROC BindBuffer = nullptr;
    PFNGLBUFFERDATAPROC BufferData = nullptr;

    PFNGLGENVERTEXARRAYSPROC GenVertexArrays = nullptr;
    PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays = nullptr;
    PFNGLBINDVERTEXARRAYPROC BindVertexArray = nullptr;
    PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray = nullptr;
    PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer = nullptr;

    PFNGLDRAWELEMENTSPROC DrawElements = nullptr;

    PFNGLGENTEXTURESPROC GenTextures = nullptr;
    PFNGLDELETETEXTURESPROC DeleteTextures = nullptr;
    PFNGLBINDTEXTUREPROC BindTexture = nullptr;
    PFNGLTEXIMAGE2DPROC TexImage2D = nullptr;
    PFNGLTEXPARAMETERIPROC TexParameteri = nullptr;
    PFNGLACTIVETEXTUREPROC ActiveTexture = nullptr;
    PFNGLPIXELSTOREIPROC PixelStorei = nullptr;

    PFNGLGENSAMPLERSPROC GenSamplers = nullptr;
    PFNGLDELETESAMPLERSPROC DeleteSamplers = nullptr;
    PFNGLSAMPLERPARAMETERIPROC SamplerParameteri = nullptr;
    PFNGLBINDSAMPLERPROC BindSampler = nullptr;

    PFNGLBLENDFUNCSEPARATEPROC BlendFuncSeparate = nullptr;
    PFNGLBLENDEQUATIONSEPARATEPROC BlendEquationSeparate = nullptr;
    PFNGLDEPTHFUNCPROC DepthFunc = nullptr;
    PFNGLDEPTHMASKPROC DepthMask = nullptr;
    PFNGLCULLFACEPROC CullFace = nullptr;
    PFNGLFRONTFACEPROC FrontFace = nullptr;
    PFNGLPOLYGONMODEPROC PolygonMode = nullptr;

    PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation = nullptr;
    PFNGLUNIFORM1IPROC Uniform1i = nullptr;
    PFNGLUNIFORM1FPROC Uniform1f = nullptr;
    PFNGLUNIFORM3FPROC Uniform3f = nullptr;
    PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv = nullptr;

    PFNGLVIEWPORTPROC Viewport = nullptr;
    PFNGLCLEARCOLORPROC ClearColor = nullptr;
    PFNGLCLEARPROC Clear = nullptr;
};

OpenGLBackend::OpenGLBackend(Win32Window& window)
    : window_(window) {
    gl_ = new Functions();
}

OpenGLBackend::~OpenGLBackend() {
    shutdown();
    delete gl_;
    gl_ = nullptr;
}

const char* OpenGLBackend::name() const noexcept {
    return "OpenGL 3.3 Core (WGL)";
}

const std::string& OpenGLBackend::last_error() const noexcept {
    return last_error_;
}

void OpenGLBackend::set_error(const std::string& message) {
    last_error_ = message;
}

OpenGLBackend::GLProc OpenGLBackend::load_function(
    const char* name
) {
    PROC proc = wglGetProcAddress(name);

    const auto value = reinterpret_cast<std::intptr_t>(proc);

    if (!proc || value == 1 || value == 2 ||
        value == 3 || value == -1) {
        HMODULE module = GetModuleHandleW(L"opengl32.dll");

        if (!module) {
            module = LoadLibraryW(L"opengl32.dll");
        }

        if (module) {
            proc = reinterpret_cast<PROC>(
                GetProcAddress(module, name)
            );
        }
    }

    return proc;
}

bool OpenGLBackend::create_context() {
    hwnd_ = window_.native_handle();

    if (!hwnd_) {
        set_error("The Win32 window has no native handle.");
        return false;
    }

    device_context_ = GetDC(hwnd_);

    if (!device_context_) {
        set_error("GetDC failed for the Sao-2X window.");
        return false;
    }

    PIXELFORMATDESCRIPTOR descriptor{};
    descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    descriptor.nVersion = 1;
    descriptor.dwFlags =
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER;

    descriptor.iPixelType = PFD_TYPE_RGBA;
    descriptor.cColorBits = 32;
    descriptor.cAlphaBits = 8;
    descriptor.cDepthBits = 24;
    descriptor.cStencilBits = 8;
    descriptor.iLayerType = PFD_MAIN_PLANE;

    const int pixel_format = ChoosePixelFormat(
        device_context_,
        &descriptor
    );

    if (pixel_format == 0) {
        set_error("ChoosePixelFormat failed.");
        return false;
    }

    if (!SetPixelFormat(
        device_context_,
        pixel_format,
        &descriptor
    )) {
        set_error("SetPixelFormat failed.");
        return false;
    }

    HGLRC temporary_context = wglCreateContext(device_context_);

    if (!temporary_context) {
        set_error("Failed to create the temporary WGL context.");
        return false;
    }

    if (!wglMakeCurrent(device_context_, temporary_context)) {
        wglDeleteContext(temporary_context);
        set_error("Failed to activate the temporary WGL context.");
        return false;
    }

    auto create_context_function =
        reinterpret_cast<WglCreateContextAttribsARB>(
            wglGetProcAddress("wglCreateContextAttribsARB")
        );

    if (!create_context_function) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(temporary_context);

        set_error(
            "WGL_ARB_create_context is unavailable. "
            "OpenGL 3.3 Core cannot be created."
        );

        return false;
    }

    const int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    context_ = create_context_function(
        device_context_,
        nullptr,
        attributes
    );

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(temporary_context);

    if (!context_) {
        set_error("Failed to create an OpenGL 3.3 Core context.");
        return false;
    }

    if (!wglMakeCurrent(device_context_, context_)) {
        set_error("Failed to activate the OpenGL 3.3 context.");
        return false;
    }

    return true;
}

#define SAO_LOAD_GL(member, type, symbol)                    \
    gl_->member = reinterpret_cast<type>(load_function(symbol)); \
    if (!gl_->member) {                                      \
        set_error("Failed to load OpenGL function: " symbol); \
        return false;                                        \
    }

bool OpenGLBackend::load_functions() {
    SAO_LOAD_GL(CreateShader, PFNGLCREATESHADERPROC, "glCreateShader");
    SAO_LOAD_GL(ShaderSource, PFNGLSHADERSOURCEPROC, "glShaderSource");
    SAO_LOAD_GL(CompileShader, PFNGLCOMPILESHADERPROC, "glCompileShader");
    SAO_LOAD_GL(GetShaderiv, PFNGLGETSHADERIVPROC, "glGetShaderiv");
    SAO_LOAD_GL(GetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC, "glGetShaderInfoLog");
    SAO_LOAD_GL(DeleteShader, PFNGLDELETESHADERPROC, "glDeleteShader");

    SAO_LOAD_GL(CreateProgram, PFNGLCREATEPROGRAMPROC, "glCreateProgram");
    SAO_LOAD_GL(AttachShader, PFNGLATTACHSHADERPROC, "glAttachShader");
    SAO_LOAD_GL(LinkProgram, PFNGLLINKPROGRAMPROC, "glLinkProgram");
    SAO_LOAD_GL(GetProgramiv, PFNGLGETPROGRAMIVPROC, "glGetProgramiv");
    SAO_LOAD_GL(GetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC, "glGetProgramInfoLog");
    SAO_LOAD_GL(DeleteProgram, PFNGLDELETEPROGRAMPROC, "glDeleteProgram");
    SAO_LOAD_GL(UseProgram, PFNGLUSEPROGRAMPROC, "glUseProgram");

    SAO_LOAD_GL(GenBuffers, PFNGLGENBUFFERSPROC, "glGenBuffers");
    SAO_LOAD_GL(DeleteBuffers, PFNGLDELETEBUFFERSPROC, "glDeleteBuffers");
    SAO_LOAD_GL(BindBuffer, PFNGLBINDBUFFERPROC, "glBindBuffer");
    SAO_LOAD_GL(BufferData, PFNGLBUFFERDATAPROC, "glBufferData");

    SAO_LOAD_GL(GenVertexArrays, PFNGLGENVERTEXARRAYSPROC, "glGenVertexArrays");
    SAO_LOAD_GL(DeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC, "glDeleteVertexArrays");
    SAO_LOAD_GL(BindVertexArray, PFNGLBINDVERTEXARRAYPROC, "glBindVertexArray");
    SAO_LOAD_GL(EnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC, "glEnableVertexAttribArray");
    SAO_LOAD_GL(VertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC, "glVertexAttribPointer");

    SAO_LOAD_GL(DrawElements, PFNGLDRAWELEMENTSPROC, "glDrawElements");

    SAO_LOAD_GL(GenTextures, PFNGLGENTEXTURESPROC, "glGenTextures");
    SAO_LOAD_GL(DeleteTextures, PFNGLDELETETEXTURESPROC, "glDeleteTextures");
    SAO_LOAD_GL(BindTexture, PFNGLBINDTEXTUREPROC, "glBindTexture");
    SAO_LOAD_GL(TexImage2D, PFNGLTEXIMAGE2DPROC, "glTexImage2D");
    SAO_LOAD_GL(TexParameteri, PFNGLTEXPARAMETERIPROC, "glTexParameteri");
    SAO_LOAD_GL(ActiveTexture, PFNGLACTIVETEXTUREPROC, "glActiveTexture");
    SAO_LOAD_GL(PixelStorei, PFNGLPIXELSTOREIPROC, "glPixelStorei");

    SAO_LOAD_GL(GenSamplers, PFNGLGENSAMPLERSPROC, "glGenSamplers");
    SAO_LOAD_GL(DeleteSamplers, PFNGLDELETESAMPLERSPROC, "glDeleteSamplers");
    SAO_LOAD_GL(SamplerParameteri, PFNGLSAMPLERPARAMETERIPROC, "glSamplerParameteri");
    SAO_LOAD_GL(BindSampler, PFNGLBINDSAMPLERPROC, "glBindSampler");

    SAO_LOAD_GL(BlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC, "glBlendFuncSeparate");
    SAO_LOAD_GL(BlendEquationSeparate, PFNGLBLENDEQUATIONSEPARATEPROC, "glBlendEquationSeparate");
    SAO_LOAD_GL(DepthFunc, PFNGLDEPTHFUNCPROC, "glDepthFunc");
    SAO_LOAD_GL(DepthMask, PFNGLDEPTHMASKPROC, "glDepthMask");
    SAO_LOAD_GL(CullFace, PFNGLCULLFACEPROC, "glCullFace");
    SAO_LOAD_GL(FrontFace, PFNGLFRONTFACEPROC, "glFrontFace");
    SAO_LOAD_GL(PolygonMode, PFNGLPOLYGONMODEPROC, "glPolygonMode");

    SAO_LOAD_GL(GetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC, "glGetUniformLocation");
    SAO_LOAD_GL(Uniform1i, PFNGLUNIFORM1IPROC, "glUniform1i");
    SAO_LOAD_GL(Uniform1f, PFNGLUNIFORM1FPROC, "glUniform1f");
    SAO_LOAD_GL(Uniform3f, PFNGLUNIFORM3FPROC, "glUniform3f");
    SAO_LOAD_GL(UniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC, "glUniformMatrix4fv");

    SAO_LOAD_GL(Viewport, PFNGLVIEWPORTPROC, "glViewport");
    SAO_LOAD_GL(ClearColor, PFNGLCLEARCOLORPROC, "glClearColor");
    SAO_LOAD_GL(Clear, PFNGLCLEARPROC, "glClear");

    return true;
}

#undef SAO_LOAD_GL

bool OpenGLBackend::initialize() {
    if (initialized_) {
        return true;
    }

    last_error_.clear();

    if (!create_context()) {
        shutdown();
        return false;
    }

    if (!load_functions()) {
        shutdown();
        return false;
    }

    const char* version = reinterpret_cast<const char*>(
        glGetString(GL_VERSION)
    );

    if (!version) {
        set_error("OpenGL did not return a version string.");
        shutdown();
        return false;
    }

    sampler_objects_.resize(16, 0);

    initialized_ = true;

    set_viewport(
        window_.size().width,
        window_.size().height
    );

    return true;
}

void OpenGLBackend::shutdown() {
    if (context_ && device_context_) {
        wglMakeCurrent(device_context_, context_);

        if (gl_ && gl_->DeleteSamplers) {
            for (unsigned int sampler : sampler_objects_) {
                if (sampler != 0) {
                    gl_->DeleteSamplers(1, &sampler);
                }
            }
        }

        sampler_objects_.clear();

        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(context_);
        context_ = nullptr;
    }

    if (device_context_ && hwnd_) {
        ReleaseDC(hwnd_, device_context_);
    }

    device_context_ = nullptr;
    hwnd_ = nullptr;
    initialized_ = false;
}

bool OpenGLBackend::make_current() {
    if (!context_ || !device_context_) {
        return false;
    }

    return wglMakeCurrent(device_context_, context_) == TRUE;
}

void OpenGLBackend::present() {
    if (!initialized_ || !make_current()) {
        return;
    }

    SwapBuffers(device_context_);
}

void OpenGLBackend::set_viewport(int width, int height) {
    if (!initialized_ || !make_current() || width <= 0 || height <= 0) {
        return;
    }

    gl_->Viewport(0, 0, width, height);
}

void OpenGLBackend::clear(
    float r,
    float g,
    float b,
    float a
) {
    if (!initialized_ || !make_current()) {
        return;
    }

    gl_->ClearColor(r, g, b, a);
    gl_->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool OpenGLBackend::compile_shader(
    ShaderStage stage,
    const std::string& source,
    std::uint32_t& shader_handle,
    std::string& log
) {
    shader_handle = 0;

    if (!initialized_ || !make_current()) {
        log = "OpenGL context is not active.";
        return false;
    }

    const GLenum gl_stage = shader_stage_to_gl(stage);

    if (gl_stage == 0) {
        log = "This OpenGL backend does not support compute shaders.";
        return false;
    }

    const GLuint shader = gl_->CreateShader(gl_stage);

    if (!shader) {
        log = "glCreateShader failed.";
        return false;
    }

    const char* source_pointer = source.c_str();

    gl_->ShaderSource(shader, 1, &source_pointer, nullptr);
    gl_->CompileShader(shader);

    GLint status = 0;
    gl_->GetShaderiv(shader, GL_COMPILE_STATUS, &status);

    GLint log_length = 0;
    gl_->GetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 1) {
        log.resize(static_cast<std::size_t>(log_length));

        GLsizei written = 0;

        gl_->GetShaderInfoLog(
            shader,
            log_length,
            &written,
            &log[0]
        );

        log.resize(static_cast<std::size_t>(
            std::max<GLsizei>(0, written)
        ));
    } else {
        log.clear();
    }

    if (!status) {
        gl_->DeleteShader(shader);
        return false;
    }

    shader_handle = shader;
    return true;
}

void OpenGLBackend::destroy_shader(std::uint32_t handle) {
    if (initialized_ && handle && make_current()) {
        gl_->DeleteShader(handle);
    }
}

bool OpenGLBackend::create_program(
    std::uint32_t& program_handle,
    std::string& log
) {
    program_handle = 0;

    if (!initialized_ || !make_current()) {
        log = "OpenGL context is not active.";
        return false;
    }

    program_handle = gl_->CreateProgram();

    if (!program_handle) {
        log = "glCreateProgram failed.";
        return false;
    }

    log.clear();
    return true;
}

bool OpenGLBackend::attach_shader(
    std::uint32_t program_handle,
    std::uint32_t shader_handle,
    std::string& log
) {
    if (!initialized_ || !make_current() ||
        !program_handle || !shader_handle) {
        log = "Invalid OpenGL program or shader handle.";
        return false;
    }

    gl_->AttachShader(program_handle, shader_handle);
    log.clear();
    return true;
}

bool OpenGLBackend::link_program(
    std::uint32_t program_handle,
    std::string& log
) {
    if (!initialized_ || !make_current() || !program_handle) {
        log = "Invalid OpenGL program handle.";
        return false;
    }

    gl_->LinkProgram(program_handle);

    GLint status = 0;
    gl_->GetProgramiv(program_handle, GL_LINK_STATUS, &status);

    GLint log_length = 0;
    gl_->GetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 1) {
        log.resize(static_cast<std::size_t>(log_length));

        GLsizei written = 0;

        gl_->GetProgramInfoLog(
            program_handle,
            log_length,
            &written,
            &log[0]
        );

        log.resize(static_cast<std::size_t>(
            std::max<GLsizei>(0, written)
        ));
    } else {
        log.clear();
    }

    return status != 0;
}

void OpenGLBackend::destroy_program(std::uint32_t handle) {
    if (initialized_ && handle && make_current()) {
        gl_->DeleteProgram(handle);
    }
}

void OpenGLBackend::use_program(std::uint32_t handle) {
    if (initialized_ && make_current()) {
        gl_->UseProgram(handle);
    }
}

void OpenGLBackend::apply_render_state(const RenderState& state) {
    if (!initialized_ || !make_current()) {
        return;
    }

    if (state.depth_test_enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    gl_->DepthMask(state.depth_write_enabled ? GL_TRUE : GL_FALSE);
    gl_->DepthFunc(depth_function_to_gl(state.depth_function));

    if (state.face_culling_enabled &&
        state.cull_mode != CullMode::None) {
        glEnable(GL_CULL_FACE);

        GLenum mode = GL_BACK;

        if (state.cull_mode == CullMode::Front) {
            mode = GL_FRONT;
        } else if (state.cull_mode == CullMode::FrontAndBack) {
            mode = GL_FRONT_AND_BACK;
        }

        gl_->CullFace(mode);
    } else {
        glDisable(GL_CULL_FACE);
    }

    gl_->FrontFace(
        state.front_face == FrontFace::Clockwise
            ? GL_CW
            : GL_CCW
    );

    if (state.blending.enabled) {
        glEnable(GL_BLEND);

        gl_->BlendFuncSeparate(
            blend_factor_to_gl(state.blending.source_color),
            blend_factor_to_gl(state.blending.destination_color),
            blend_factor_to_gl(state.blending.source_alpha),
            blend_factor_to_gl(state.blending.destination_alpha)
        );

        gl_->BlendEquationSeparate(
            blend_operation_to_gl(state.blending.color_operation),
            blend_operation_to_gl(state.blending.alpha_operation)
        );
    } else {
        glDisable(GL_BLEND);
    }

    gl_->PolygonMode(
        GL_FRONT_AND_BACK,
        state.wireframe ? GL_LINE : GL_FILL
    );
}

void OpenGLBackend::apply_texture_sampler(
    unsigned int texture_unit,
    const TextureSampler& sampler
) {
    if (!initialized_ || !make_current() ||
        texture_unit >= sampler_objects_.size()) {
        return;
    }

    unsigned int& handle = sampler_objects_[texture_unit];

    if (!handle) {
        gl_->GenSamplers(1, &handle);
    }

    if (!handle) {
        return;
    }

    gl_->SamplerParameteri(
        handle,
        GL_TEXTURE_MIN_FILTER,
        static_cast<GLint>(texture_filter_to_gl(sampler.min_filter))
    );

    gl_->SamplerParameteri(
        handle,
        GL_TEXTURE_MAG_FILTER,
        static_cast<GLint>(texture_filter_to_gl(sampler.mag_filter))
    );

    gl_->SamplerParameteri(
        handle,
        GL_TEXTURE_WRAP_S,
        static_cast<GLint>(texture_wrap_to_gl(sampler.wrap_u))
    );

    gl_->SamplerParameteri(
        handle,
        GL_TEXTURE_WRAP_T,
        static_cast<GLint>(texture_wrap_to_gl(sampler.wrap_v))
    );

    gl_->BindSampler(texture_unit, handle);
}

std::uint32_t OpenGLBackend::create_buffer() {
    if (!initialized_ || !make_current()) {
        return 0;
    }

    GLuint handle = 0;
    gl_->GenBuffers(1, &handle);
    return handle;
}

void OpenGLBackend::destroy_buffer(std::uint32_t buffer) {
    if (initialized_ && buffer && make_current()) {
        gl_->DeleteBuffers(1, &buffer);
    }
}

bool OpenGLBackend::upload_array_buffer(
    std::uint32_t buffer,
    const void* data,
    std::size_t size,
    bool dynamic
) {
    if (!initialized_ || !make_current() || !buffer ||
        size > static_cast<std::size_t>(
            std::numeric_limits<GLsizeiptr>::max()
        )) {
        return false;
    }

    gl_->BindBuffer(GL_ARRAY_BUFFER, buffer);

    gl_->BufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(size),
        data,
        dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW
    );

    return true;
}

bool OpenGLBackend::upload_element_buffer(
    std::uint32_t buffer,
    const void* data,
    std::size_t size,
    bool dynamic
) {
    if (!initialized_ || !make_current() || !buffer ||
        size > static_cast<std::size_t>(
            std::numeric_limits<GLsizeiptr>::max()
        )) {
        return false;
    }

    gl_->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);

    gl_->BufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(size),
        data,
        dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW
    );

    return true;
}

std::uint32_t OpenGLBackend::create_vertex_array() {
    if (!initialized_ || !make_current()) {
        return 0;
    }

    GLuint handle = 0;
    gl_->GenVertexArrays(1, &handle);
    return handle;
}

void OpenGLBackend::destroy_vertex_array(std::uint32_t vertex_array) {
    if (initialized_ && vertex_array && make_current()) {
        gl_->DeleteVertexArrays(1, &vertex_array);
    }
}

void OpenGLBackend::bind_vertex_array(std::uint32_t vertex_array) {
    if (initialized_ && make_current()) {
        gl_->BindVertexArray(vertex_array);
    }
}

void OpenGLBackend::configure_vertex_attribute(
    unsigned int index,
    int component_count,
    std::size_t stride,
    std::size_t offset
) {
    if (!initialized_ || !make_current()) {
        return;
    }

    gl_->EnableVertexAttribArray(index);

    gl_->VertexAttribPointer(
        index,
        component_count,
        GL_FLOAT,
        GL_FALSE,
        static_cast<GLsizei>(stride),
        reinterpret_cast<const void*>(offset)
    );
}

void OpenGLBackend::draw_indexed(std::uint32_t index_count) {
    if (!initialized_ || !make_current() || index_count == 0) {
        return;
    }

    gl_->DrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(index_count),
        GL_UNSIGNED_INT,
        nullptr
    );
}

std::uint32_t OpenGLBackend::create_texture_2d() {
    if (!initialized_ || !make_current()) {
        return 0;
    }

    GLuint handle = 0;
    gl_->GenTextures(1, &handle);
    return handle;
}

void OpenGLBackend::destroy_texture(std::uint32_t texture) {
    if (initialized_ && texture && make_current()) {
        gl_->DeleteTextures(1, &texture);
    }
}

bool OpenGLBackend::upload_texture_2d(
    std::uint32_t texture,
    int width,
    int height,
    const std::uint8_t* rgba_pixels
) {
    if (!initialized_ || !make_current() || !texture ||
        width <= 0 || height <= 0 || !rgba_pixels) {
        return false;
    }

    gl_->BindTexture(GL_TEXTURE_2D, texture);
    gl_->PixelStorei(GL_UNPACK_ALIGNMENT, 1);

    gl_->TexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgba_pixels
    );

    return true;
}

void OpenGLBackend::bind_texture_2d(
    unsigned int texture_unit,
    std::uint32_t texture
) {
    if (!initialized_ || !make_current()) {
        return;
    }

    gl_->ActiveTexture(GL_TEXTURE0 + texture_unit);
    gl_->BindTexture(GL_TEXTURE_2D, texture);
}

int OpenGLBackend::uniform_location(
    std::uint32_t program,
    const char* name
) {
    if (!initialized_ || !make_current() || !name) {
        return -1;
    }

    return gl_->GetUniformLocation(program, name);
}

void OpenGLBackend::uniform_int(int location, int value) {
    if (initialized_ && location >= 0 && make_current()) {
        gl_->Uniform1i(location, value);
    }
}

void OpenGLBackend::uniform_float(int location, float value) {
    if (initialized_ && location >= 0 && make_current()) {
        gl_->Uniform1f(location, value);
    }
}

void OpenGLBackend::uniform_vec3(
    int location,
    float x,
    float y,
    float z
) {
    if (initialized_ && location >= 0 && make_current()) {
        gl_->Uniform3f(location, x, y, z);
    }
}

void OpenGLBackend::uniform_mat4(
    int location,
    const float* column_major_matrix
) {
    if (initialized_ && location >= 0 &&
        column_major_matrix && make_current()) {
        gl_->UniformMatrix4fv(
            location,
            1,
            GL_FALSE,
            column_major_matrix
        );
    }
}

const char* OpenGLBackend::version_string() const noexcept {
    if (!initialized_) {
        return "";
    }

    return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

const char* OpenGLBackend::renderer_string() const noexcept {
    if (!initialized_) {
        return "";
    }

    return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

} // namespace sao

#endif
