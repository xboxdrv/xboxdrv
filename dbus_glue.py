#!/usr/bin/env python3

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
import os
import argparse
import subprocess
import re


def build_dbus_glue(target, source, dbus_prefix):
    """
    C++ doesn't allow casting from void* to a function pointer,
    thus we have to change the code to use a union to do the
    conversion.
    """
    # dbus-binding-tool shells out to glib-genmarshal with the deprecated
    # "--header --body" pair. Prefer a PATH shim that rewrites the flags to
    # "--body --prototypes". Also filter the known deprecation warning from
    # stderr in case the tool invokes genmarshal by absolute path (nix) and
    # bypasses PATH.
    env = os.environ.copy()
    tools_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tools")
    shim_dir = os.path.join(os.path.dirname(os.path.abspath(target)) if target else ".",
                            ".dbus-shim")
    os.makedirs(shim_dir, exist_ok=True)
    shim = os.path.join(shim_dir, "glib-genmarshal")
    wrapper = os.path.abspath(os.path.join(tools_dir, "glib-genmarshal-wrapper"))
    if os.path.lexists(shim):
        os.unlink(shim)
    os.symlink(wrapper, shim)
    os.chmod(wrapper, 0o755)
    env["PATH"] = shim_dir + os.pathsep + env.get("PATH", "")

    proc = subprocess.Popen(["dbus-binding-tool",
                             "--mode=glib-server",
                             "--prefix=" + dbus_prefix, source],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            env=env)
    out, err = proc.communicate()
    if proc.returncode != 0:
        sys.stderr.write(err.decode(errors="replace"))
        raise subprocess.CalledProcessError(proc.returncode, "dbus-binding-tool")
    for line in err.decode(errors="replace").splitlines():
        if "Using --header and --body at the same time is deprecated" in line:
            continue
        if line.strip():
            sys.stderr.write(line + "\n")
    xml = out.decode()
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
