#include <print>
#include <source_location>

#include "trace.hpp"

const char* level_to_str(const Trace::Level level) {
    switch (level) {
        case Trace::Level::NONE:
            return "NONE";
        case Trace::Level::INFO:
            return "INFO";
        case Trace::Level::DEBUG:
            return "DEBUG";
    }
}

namespace Trace {
Level current_level { Level::INFO };

void trace(const Level level, const char* msg, const std::source_location src_loc) {
    if (level == current_level) {
        std::println(
                "[{}] {}:{}: {}",
                level_to_str(level),
                src_loc.file_name(),
                src_loc.line(),
                msg
                );
    }
}
};
