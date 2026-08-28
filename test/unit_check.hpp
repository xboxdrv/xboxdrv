/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#ifndef HEADER_XBOXDRV_TEST_UNIT_CHECK_HPP
#define HEADER_XBOXDRV_TEST_UNIT_CHECK_HPP

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

/** Minimal check helpers — no external test framework dependency. */

namespace xboxdrv {
namespace test {

inline int& failure_count()
{
  static int n = 0;
  return n;
}

inline void check_failed(char const* file, int line, std::string const& msg)
{
  std::cerr << file << ":" << line << ": check failed: " << msg << "\n";
  failure_count() += 1;
}

template <typename A, typename B>
inline void check_eq(char const* file, int line,
                     char const* expr_a, A const& a,
                     char const* expr_b, B const& b)
{
  if (!(a == b))
  {
    std::ostringstream out;
    out << expr_a << " (" << a << ") == " << expr_b << " (" << b << ")";
    check_failed(file, line, out.str());
  }
}

inline void check_true(char const* file, int line, char const* expr, bool v)
{
  if (!v)
  {
    check_failed(file, line, std::string(expr) + " is false");
  }
}

inline int exit_code()
{
  return failure_count() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace test
} // namespace xboxdrv

#define CHECK_EQ(a, b) \
  ::xboxdrv::test::check_eq(__FILE__, __LINE__, #a, (a), #b, (b))
#define CHECK_TRUE(expr) \
  ::xboxdrv::test::check_true(__FILE__, __LINE__, #expr, static_cast<bool>(expr))

#endif

/* EOF */
