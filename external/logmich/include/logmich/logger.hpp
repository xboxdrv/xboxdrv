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

#ifndef HEADER_LOGMICH_LOGGER_HPP
#define HEADER_LOGMICH_LOGGER_HPP

#include <iostream>
#include <string_view>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace logmich {
namespace detail {

/** Takes __PRETTY_FUNCTION__ and tries to shorten it to the form:
    Classname::function() */
std::string_view log_pretty_print(std::string_view str);

} // namespace detail

enum class LogLevel
{
  /** Used in set_log_level() to disable all output */
  NONE,

  /** For fatal errors that will likely lead to program termination */
  FATAL,

  /** For errors that shouldn't happen */
  ERROR,

  /** For mild errors that aren't critical */
  WARNING,

  /** informal status messages that don't indicate a fault in the
      program */
  INFO,

  /** extra verbose informal debug messages */
  DEBUG,

  /** temporary extra verbose debugging messages */
  TRACE,
};

class Logger
{
private:
  LogLevel m_log_level;

public:
  Logger();
  void incr_log_level(LogLevel level);
  void set_log_level(LogLevel level);
  LogLevel get_log_level() const;

  void append(std::ostream& out, LogLevel level, std::string_view file, int line, std::string_view msg);
  void append(LogLevel level, std::string_view file, int line, std::string_view msg);

  void append_format(LogLevel level, std::string_view file, int line, std::string_view msg = {}) {
    append(level, file, line, msg);
  }

  template<typename ...Args>
  void append_format(LogLevel level, std::string_view file, int line, std::string_view fmt, Args&&... args)
  {
    try {
      append(level, file, line, fmt::format(fmt::runtime(fmt), args...));
    } catch (std::exception const& err) {
      std::cerr << "[LOG ERROR] " << file << ":" << line << ": " << err.what() << ": \"" << fmt << "\"" << std::endl;
    } catch (...) {
      std::cerr << "[LOG ERROR] unknown exception" << std::endl;
    }
  }
};

} // namespace logmich

#endif

/* EOF */
