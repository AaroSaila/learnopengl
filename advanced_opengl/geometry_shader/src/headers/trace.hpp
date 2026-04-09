#pragma once

#include <source_location>

namespace Trace {
enum class Level {
    INFO,
    DEBUG
};

extern Level current_level;

void trace(const Level level, const char* msg, const std::source_location src_loc = std::source_location::current());
};
