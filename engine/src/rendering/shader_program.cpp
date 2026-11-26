
#include "sao/rendering/shader_program.hpp"

namespace sao {

bool ShaderProgram::is_linked() const noexcept {
    return linked_;
}

std::uint32_t ShaderProgram::handle() const noexcept {
    return handle_;
}

const std::string& ShaderProgram::log() const noexcept {
    return log_;
}

} // namespace sao
