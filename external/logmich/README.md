logmich - A Trivial Logging Library
===================================

logmich is a trivial logging library for C++. It lacks anything but
the bare essentials and allows a `printf` like syntax for message
formating.

Usage is as follows:

    // Set the log level at which we want to log
    logmich::set_log_level(logmich::kInfo);

    // output some log messages
    log_error("error level log message, number %d", 5);
    log_warn("warring level log message");
    log_info("info level log message: %s", "filename.jpg");
    log_debug("debug level log message [invisible]");
    log_tmp("tmp level log message [invisible]");

The format strings are handled by
[boost-format](http://www.boost.org/doc/libs/1_57_0/libs/format/),
thus the same rules as for boost-format apply for logmich.
