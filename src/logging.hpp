#pragma once

namespace Logging {

enum struct Level {
    Trace,
    Info,
    Debug,
    Warn,
    Error,
    None
};

[[nodiscard]] constexpr const char* LevelToString(const Level val)
{
    switch (val) {
    case Level::Trace: {
        return "LogLevel::Trace";
    }
    case Level::Info: {
        return "LogLevel::Info";
    }
    case Level::Debug: {
        return "LogLevel::Debug";
    }
    case Level::Warn: {
        return "LogLevel::Warn";
    }
    case Level::Error: {
        return "LogLevel::Error";
    }
    case Level::None: {
        return "LogLevel::None";
    }
    default: {
        return "unknown LogLevel";
    }
    }
}

static constexpr Level GlobalLevel = Level::Trace;

} // namespace Logging

#define LOG(level, ...)                                                                           \
    if constexpr (level >= Logging::GlobalLevel) {                                                \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(level), fmt::format(__VA_ARGS__)); \
    }

#define LOGT(...) LOG(Logging::Level::Trace, __VA_ARGS__)
#define LOGI(...) LOG(Logging::Level::Info, __VA_ARGS__)
#define LOGW(...) LOG(Logging::Level::Warn, __VA_ARGS__)
#define LOGD(...) LOG(Logging::Level::Debug, __VA_ARGS__)
#define LOGE(...) LOG(Logging::Level::Error, __VA_ARGS__)
#define LOGN(...) LOG(Logging::Level::None, __VA_ARGS__)
