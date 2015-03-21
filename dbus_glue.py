#!/usr/bin/env python2

# Xbox360 USB Gamepad Userspace Driver
# Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import sys
import argparse
import subprocess
import re


def build_dbus_glue(target, source, dbus_prefix):
    """
    C++ doesn't allow casting from void* to a function pointer,
    thus we have to change the code to use a union to do the
    conversion.
    """
    xml = subprocess.Popen(["dbus-binding-tool",
                            "--mode=glib-server",
                            "--prefix=" + dbus_prefix, source],
                           stdout=subprocess.PIPE).communicate()[0]

    xml = re.sub(r"callback = \(([A-Za-z_]+)\) \(marshal_data \? marshal_data : cc->callback\);",
                 r"union { \1 fn; void* obj; } conv;\n  "
                 "conv.obj = (marshal_data ? marshal_data : cc->callback);\n  "
                 "callback = conv.fn;", xml)

    with open(target, "w") as f:
        f.write(xml)


def main():
    parser = argparse.ArgumentParser(description="Generate dbus glue")
    parser.add_argument('SOURCE', action='store', nargs=1, type=str, help="SOURCE file")
    parser.add_argument('-o', '--output', metavar='TARGET', action='store', required=True, type=str, help="TARGET file")
    parser.add_argument('--dbus-prefix', metavar='PREFIX', action='store', required=True, type=str, help="Use DBus Prefix")

    args = parser.parse_args()

    build_dbus_glue(args.output, args.SOURCE[0], args.dbus_prefix)


if __name__ == "__main__":
    main()


# EOF #
