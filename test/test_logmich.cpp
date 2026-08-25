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

#include <logmich/log.hpp>

#include <ostream>

namespace {

struct custom {};

std::ostream& operator<<(std::ostream& os, const custom& c)
{
  return os << "<custom-type>";
}

} // namespace

int main()
{
  logmich::set_log_level(logmich::LogLevel::INFO);
  log_info();
  log_warn();
  log_error();
  log_debug();
  log_trace();

  log_unreachable();
  log_not_implemented();

  logmich::set_log_level(logmich::LogLevel::INFO);
  log_error("error level log message");
  log_warn("warring level log message");
  log_info("info level log message");
  log_debug("debug level log message [invisible]");
  log_trace("tmp level log message [invisible]");

  logmich::set_log_level(logmich::LogLevel::TRACE);
  log_error("error level log message with format: {}", 5);
  log_warn("warring level log message with format: {}", 10);
  log_info("info level log message with format: {}", "Hello World");
  log_debug("debug level log message with format: {} {} {} {}", 1, 2, 3, 4);
  log_trace("tmp level log message {}", 42);

  logmich::set_log_level(logmich::LogLevel::INFO);
  log_debug("this should not be visible [invisible]");
  logmich::set_log_level(logmich::LogLevel::TRACE);
  log_debug("this should be visible");
  log_trace("this should be visible");

  log_warn("format test {}", 1);
  log_warn("format test {0} {1} {0}", 1, 2);
  log_warn("format test {}", 1);
  log_warn("format test {}", 1);
  log_warn("format test {}", custom());

  logmich::set_log_level(logmich::LogLevel::NONE);
  log_info("info that shouldn't show up");
  log_warn("warning that shouldn't show up");
  log_fatal("fatal error that shouldn't show up");
  log_none("none will always show up");
  log_error("error that shouldn't show up");

  return 0;
}

/* EOF */
