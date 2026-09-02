#include <source_location>

#include "glad/glad.h"

#include "error_handling.hpp"
#include "quit.hpp"

#define CHECK_SHADER_COMPILE_ERROR(shader_id) check_shader_compile_error()

void check_shader_compile_error(const unsigned int shader_id, const char* filename, const std::source_location src_loc) {
    int success {};
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader_id, 512, nullptr, info_log);
        _log_error(src_loc.file_name(), src_loc.line(), "Shader ({}) compilation failed: {}", filename, info_log);
        quit(1);
    }
}

void check_shader_program_link_error(const unsigned int program, const std::source_location src_loc) {
    int success {};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        _log_error(src_loc.file_name(), src_loc.line(), "Linking shader program failed: {}", info_log);
        quit(-1);
    }
}

void check_framebuffer_complete(const unsigned int target, const std::source_location src_loc) {
    const GLenum res { glCheckFramebufferStatus(target) };
    if (res != GL_FRAMEBUFFER_COMPLETE) {
        _log_error(src_loc.file_name(), src_loc.line(), "Framebuffer was not complete. Status check returned 0x{:x}", res);
        quit(-1);
    }
}

void check_gl_error(const std::source_location src_loc) {
    const GLenum gl_err = glGetError();
    if (gl_err != GL_NO_ERROR) {
        _log_error(src_loc.file_name(), src_loc.line(), "[OpenGL] %d\n", gl_err);
    }
}
