#include <format>
#include <print>
#include <source_location>
#include <string>

void log_error(const char* err, std::source_location src_loc = std::source_location::current()) {
    std::println("{}:{} ERROR: {}", src_loc.file_name(), src_loc.line(), err);
}

int main() {
    log_error("an error happened");
}
