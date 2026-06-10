#pragma once

#include <array>
#include <cstddef>
#include <string_view>

enum class LogModule
{
    App,
    Config,
    Http,
    Mysql,
    Grpc,
    File,
};

namespace LogNames
{
inline constexpr std::string_view _app = "app";
inline constexpr std::string_view _config = "config";
inline constexpr std::string_view _http = "http";
inline constexpr std::string_view _mysql = "mysql";
inline constexpr std::string_view _grpc = "grpc";
inline constexpr std::string_view _file = "file";

inline constexpr std::array<std::string_view, 6> _table = {
    _app, _config, _http, _mysql, _grpc, _file,
};
} // namespace LogNames

inline std::string_view moduleName(LogModule module)
{
    return LogNames::_table[static_cast<std::size_t>(module)];
}
