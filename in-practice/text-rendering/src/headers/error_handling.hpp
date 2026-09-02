#pragma once

#include <source_location>
#include <print> // IWYU pragma: keep

#define _log_error(file, line, fmt, ...) \
    std::print(stderr, "{}:{} ERROR: ", file, line); \
    std::println(stderr, fmt __VA_OPT__(,) __VA_ARGS__)

#define log_error(fmt, ...) \
    _log_error(__FILE__, __LINE__, fmt, __VA_ARGS__);

void check_shader_compile_error(const unsigned int shader_id, const char* filename, const std::source_location src_loc = std::source_location::current());
void check_shader_program_link_error(const unsigned int program, const std::source_location src_loc = std::source_location::current());
void check_framebuffer_complete(const unsigned int target, const std::source_location src_loc = std::source_location::current());
void check_gl_error(const std::source_location src_loc = std::source_location::current());
