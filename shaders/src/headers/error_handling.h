#ifndef ERROR_HANDLING_H_
#define ERROR_HANDLING_H_

#include <source_location>

void log_error(const char* err, const std::source_location src_loc = std::source_location::current());
void check_shader_compile_error(const unsigned int shader_id, const std::source_location src_loc = std::source_location::current());
void check_shader_program_link_error(const unsigned int program, const std::source_location src_loc = std::source_location::current());

#endif // ERROR_HANDLING_H_
