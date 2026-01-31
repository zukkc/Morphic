#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// #define DISABLE_LOGS_IN_RELEASE

namespace DebugUtils {
template <typename... Args>
godot::String format_log(const godot::String &fmt, Args... args) {
  return godot::vformat(fmt, args...);
}

inline godot::String format_log(const godot::String &msg) { return msg; }
} // namespace DebugUtils

#if defined(DISABLE_LOGS_IN_RELEASE) && defined(NDEBUG)
#define GD_LOG(...)
#define GD_WARN(...)
#define GD_ERR(...)
#define GD_VAR(var)
#else

#define LOG(msg, ...)                                                          \
  godot::UtilityFunctions::print("[INFO] ", __FUNCTION__, ":", __LINE__, ": ", \
                                 DebugUtils::format_log(msg, ##__VA_ARGS__))

#define WARN(msg, ...)                                                         \
  godot::UtilityFunctions::push_warning(                                       \
      __FUNCTION__, ":", __LINE__, ": ",                                       \
      DebugUtils::format_log(msg, ##__VA_ARGS__))

#define ERR(msg, ...)                                                          \
  godot::UtilityFunctions::push_error(                                         \
      __FUNCTION__, ":", __LINE__, ": ",                                       \
      DebugUtils::format_log(msg, ##__VA_ARGS__))

#define VAR(var)                                                               \
  godot::UtilityFunctions::print("[VAR] ", __FUNCTION__, ": ", #var, " = ", var)

#endif
