// LogMich - A Trivial Logging Library
// Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef HEADER_LOGMICH_LOGMICH_HPP
#define HEADER_LOGMICH_LOGMICH_HPP

#include <string>

#include "logger.hpp"

namespace logmich {

extern Logger g_logger;

inline void incr_log_level(LogLevel level)
{
  g_logger.incr_log_level(level);
}

inline void set_log_level(LogLevel level)
{
  g_logger.set_log_level(level);
}

inline LogLevel get_log_level()
{
  return g_logger.get_log_level();
}

} // namespace logmich

#define log_trace(...) do {                                             \
    if (logmich::g_logger.get_log_level() >= logmich::LogLevel::TRACE) { \
      logmich::g_logger.append_format(logmich::LogLevel::TRACE, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    }                                                                   \
  } while(false)

#define log_debug(...) do {                                             \
    if (logmich::g_logger.get_log_level() >= logmich::LogLevel::DEBUG) { \
      logmich::g_logger.append_format(logmich::LogLevel::DEBUG, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    }                                                                   \
  } while(false)

#define log_info(...) do {                                              \
    if (logmich::g_logger.get_log_level() >= logmich::LogLevel::INFO) { \
      logmich::g_logger.append_format(logmich::LogLevel::INFO, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    }                                                                   \
  } while(false)

#define log_warn(...) do {                                              \
    if (logmich::g_logger.get_log_level() >= logmich::LogLevel::WARNING) { \
      logmich::g_logger.append_format(logmich::LogLevel::WARNING, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    }                                                                   \
  } while(false)

#define log_error(...) do {                                             \
    if (logmich::g_logger.get_log_level() >= logmich::LogLevel::ERROR) { \
      logmich::g_logger.append_format(logmich::LogLevel::ERROR, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    }                                                                   \
  } while(false)

#define log_fatal(...) do {                                             \
    if (logmich::g_logger.get_log_level() >= logmich::LogLevel::FATAL) { \
      logmich::g_logger.append_format(logmich::LogLevel::FATAL, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    }                                                                   \
  } while(false)

#define log_none(...) do {                                             \
    if (logmich::g_logger.get_log_level() >= logmich::LogLevel::NONE) { \
      logmich::g_logger.append_format(logmich::LogLevel::NONE, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    }                                                                   \
  } while(false)

#define log_unreachable() log_fatal("unreachable code reached")

#define log_not_implemented() log_error("not implemented")

#endif

/* EOF */
