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

static constexpr Level GlobalLevel = Level::Error;

#define LOG(level, ...)                                                                           \
    if constexpr (level >= Logging::GlobalLevel) {                                                      \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(level), fmt::format(__VA_ARGS__)); \
    }

} // namespace Logging

#define LOGT(...)                                                                                                 \
    if constexpr (Logging::Level::Trace >= Logging::GlobalLevel) {                                                      \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(Logging::Level::Trace), fmt::format(__VA_ARGS__)); \
    }

#define LOGI(...)                                                                                                \
    if constexpr (Logging::Level::Info >= Logging::GlobalLevel) {                                                      \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(Logging::Level::Info), fmt::format(__VA_ARGS__)); \
    }

#define LOGD(...)                                                                                                 \
    if constexpr (Logging::Level::Debug >= Logging::GlobalLevel) {                                                      \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(Logging::Level::Debug), fmt::format(__VA_ARGS__)); \
    }

#define LOGW(...)                                                                                                \
    if constexpr (Logging::Level::Warn >= Logging::GlobalLevel) {                                                      \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(Logging::Level::Warn), fmt::format(__VA_ARGS__)); \
    }

#define LOGE(...)                                                                                                 \
    if constexpr (Logging::Level::Error >= Logging::GlobalLevel) {                                                      \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(Logging::Level::Error), fmt::format(__VA_ARGS__)); \
    }

#define LOGN(...)                                                                                                \
    if constexpr (Logging::Level::None >= Logging::GlobalLevel) {                                                      \
        fmt::print(stderr, "[{}] {}\n", Logging::LevelToString(Logging::Level::None), fmt::format(__VA_ARGS__)); \
    }
