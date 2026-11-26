
#include "sao/rendering/shader.hpp"

#include <utility>

namespace sao {

Shader::Shader(ShaderStage stage, std::string source)
    : stage_(stage),
      source_(std::move(source)) {
}

void Shader::set_stage(ShaderStage stage) noexcept {
    stage_ = stage;
    compiled_ = false;
}

void Shader::set_source(std::string source) {
    source_ = std::move(source);
    compiled_ = false;
}

ShaderStage Shader::stage() const noexcept {
    return stage_;
}

const std::string& Shader::source() const noexcept {
    return source_;
}

bool Shader::is_compiled() const noexcept {
    return compiled_;
}

std::uint32_t Shader::handle() const noexcept {
    return handle_;
}

const std::string& Shader::log() const noexcept {
    return log_;
}

} // namespace sao
