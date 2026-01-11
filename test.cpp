#include <print>
#include <source_location>
#include <glm/glm.hpp>

void log_error(const char* err, std::source_location src_loc = std::source_location::current()) {
    std::println("{}:{} ERROR: {}", src_loc.file_name(), src_loc.line(), err);
}

int main() {
    glm::vec3 v { 6.9f };
    std::println("{}, {}, {}", v.x, v.y, v.z);
}
