#include <print>
#include <source_location>

#include "glad/glad.h"

#include "quit.hpp"

#define CHECK_SHADER_COMPILE_ERROR(shader_id) check_shader_compile_error()

void log_error(const char* err, const std::source_location src_loc = std::source_location::current()) {
    std::println(stderr, "{}:{} ERROR: {}", src_loc.file_name(), src_loc.line(), err);
}

void check_shader_compile_error(const unsigned int shader_id, const std::source_location src_loc = std::source_location::current()) {
    int success {};
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader_id, 512, nullptr, info_log);
        log_error(std::format("Shader compilation failed: {}", info_log).c_str(), src_loc);
        quit(1);
    }
}

void check_shader_program_link_error(const unsigned int program, const std::source_location src_loc = std::source_location::current()) {
    int success {};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        log_error(std::format("Linking shader program failed: {}", info_log).c_str(), src_loc);
        quit(-1);
    }
}
