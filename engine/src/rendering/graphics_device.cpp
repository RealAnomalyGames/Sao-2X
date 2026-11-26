
#include "sao/rendering/graphics_device.hpp"

#include <utility>

namespace sao {

GraphicsDevice::~GraphicsDevice() {
    shutdown();
}

bool GraphicsDevice::initialize(
    std::unique_ptr<GraphicsBackend> backend
) {
    if (initialized_ || !backend) {
        return false;
    }

    backend_ = std::move(backend);

    if (!backend_->initialize()) {
        backend_->shutdown();
        backend_.reset();
        return false;
    }

    initialized_ = true;
    return true;
}

void GraphicsDevice::shutdown() {
    if (backend_) {
        backend_->shutdown();
        backend_.reset();
    }

    initialized_ = false;
}

bool GraphicsDevice::is_initialized() const noexcept {
    return initialized_;
}

const char* GraphicsDevice::backend_name() const noexcept {
    return backend_ ? backend_->name() : "None";
}

bool GraphicsDevice::compile_shader(Shader& shader) {
    if (!initialized_ || shader.source_.empty()) {
        return false;
    }

    if (shader.handle_ != 0) {
        backend_->destroy_shader(shader.handle_);
        shader.handle_ = 0;
    }

    shader.compiled_ = false;
    shader.log_.clear();

    std::uint32_t handle = 0;

    const bool success = backend_->compile_shader(
        shader.stage_,
        shader.source_,
        handle,
        shader.log_
    );

    if (!success || handle == 0) {
        shader.log_ = shader.log_.empty()
            ? "Shader compilation failed."
            : shader.log_;
        return false;
    }

    shader.handle_ = handle;
    shader.compiled_ = true;

    return true;
}

bool GraphicsDevice::link_program(
    ShaderProgram& program,
    const std::vector<const Shader*>& shaders
) {
    if (!initialized_ || shaders.empty()) {
        return false;
    }

    for (const Shader* shader : shaders) {
        if (!shader || !shader->compiled_ || shader->handle_ == 0) {
            program.log_ = "All shaders must be compiled before linking.";
            program.linked_ = false;
            return false;
        }
    }

    if (program.handle_ != 0) {
        backend_->destroy_program(program.handle_);
        program.handle_ = 0;
    }

    program.linked_ = false;
    program.log_.clear();

    std::uint32_t handle = 0;

    if (!backend_->create_program(handle, program.log_) || handle == 0) {
        program.log_ = program.log_.empty()
            ? "Could not create shader program."
            : program.log_;
        return false;
    }

    for (const Shader* shader : shaders) {
        if (!backend_->attach_shader(
                handle,
                shader->handle_,
                program.log_)) {
            backend_->destroy_program(handle);
            program.log_ = program.log_.empty()
                ? "Could not attach shader."
                : program.log_;
            return false;
        }
    }

    if (!backend_->link_program(handle, program.log_)) {
        backend_->destroy_program(handle);
        program.log_ = program.log_.empty()
            ? "Shader program linking failed."
            : program.log_;
        return false;
    }

    program.handle_ = handle;
    program.linked_ = true;

    return true;
}

void GraphicsDevice::use_program(const ShaderProgram* program) {
    if (!initialized_) {
        return;
    }

    if (program && program->linked_) {
        backend_->use_program(program->handle_);
    } else {
        backend_->use_program(0);
    }
}

void GraphicsDevice::destroy_shader(Shader& shader) {
    if (initialized_ && shader.handle_ != 0) {
        backend_->destroy_shader(shader.handle_);
    }

    shader.handle_ = 0;
    shader.compiled_ = false;
}

void GraphicsDevice::destroy_program(ShaderProgram& program) {
    if (initialized_ && program.handle_ != 0) {
        backend_->destroy_program(program.handle_);
    }

    program.handle_ = 0;
    program.linked_ = false;
}

void GraphicsDevice::set_render_state(const RenderState& state) {
    if (initialized_) {
        backend_->apply_render_state(state);
    }
}

void GraphicsDevice::set_texture_sampler(
    unsigned int texture_unit,
    const TextureSampler& sampler
) {
    if (initialized_) {
        backend_->apply_texture_sampler(texture_unit, sampler);
    }
}

} // namespace sao
