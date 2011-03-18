/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEADER_XBOXDRV_LOG_HPP
#define HEADER_XBOXDRV_LOG_HPP

#include <string>
#include <sstream>

/** Takes __PRETTY_FUNCTION__ and tries to shorten it to the form:
    Classname::function() */
std::string log_pretty_print(const std::string& str);

class Logger
{
public:
  enum LogLevel {
    /** things that shouldn't happen (i.e. a catched exceptions) */
    kError,

    /** messages that indicate an recoverable error (i.e. a catched
        exceptions) */
    kWarning,

    /** informal status messages that don't indicate a fault in the
        program */
    kInfo,

    /** extra verbose debugging messages */
    kDebug
  };

private:
  LogLevel m_log_level;

public:
  Logger();
  void incr_log_level(LogLevel level);
  void set_log_level(LogLevel level);
  LogLevel get_log_level() const;
  void append(LogLevel level, const std::string& str);
  void append_unchecked(LogLevel level, const std::string& str);

};

#define log_debug(text) do { \
  if (g_logger.get_log_level() >= Logger::kDebug) \
  { \
    std::ostringstream x6ac1c382;             \
    x6ac1c382 << log_pretty_print(__PRETTY_FUNCTION__) << ": " << text; \
    g_logger.append_unchecked(Logger::kDebug, x6ac1c382.str()); \
  } \
} while(false)

#define log_info(text) do { \
  if (g_logger.get_log_level() >= Logger::kInfo) \
  { \
    std::ostringstream x6ac1c382;             \
    x6ac1c382 << log_pretty_print(__PRETTY_FUNCTION__) << ": " << text; \
    g_logger.append_unchecked(Logger::kInfo, x6ac1c382.str()); \
  } \
} while(false)

#define log_warn(text) do { \
  if (g_logger.get_log_level() >= Logger::kWarning) \
  { \
    std::ostringstream x6ac1c382;             \
    x6ac1c382 << log_pretty_print(__PRETTY_FUNCTION__) << ": " << text; \
    g_logger.append_unchecked(Logger::kWarning, x6ac1c382.str()); \
  } \
} while(false)

#define log_error(text) do { \
  if (g_logger.get_log_level() >= Logger::kError) \
  { \
    std::ostringstream x6ac1c382;             \
    x6ac1c382 << log_pretty_print(__PRETTY_FUNCTION__) << ": " << text; \
    g_logger.append_unchecked(Logger::kError, x6ac1c382.str()); \
  } \
} while(false)

/** Write an empty debug level message, thus class and function name
    are visible */
#define log_trace() do { \
  if (g_logger.get_log_level() >= Logger::kDebug) \
  { \
    std::ostringstream x6ac1c382;             \
    x6ac1c382 << log_pretty_print(__PRETTY_FUNCTION__); \
    g_logger.append_unchecked(Logger::kDebug, x6ac1c382.str()); \
  } \
} while(false)

extern Logger g_logger;

#endif

/* EOF */
