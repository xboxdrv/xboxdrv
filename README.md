> [!WARNING]
> **xboxdrv is mostly obsolete for everyday use.** Prefer the in-kernel
> `xpad` driver and/or Steam Input. This tree remains useful for heavy
> remapping, userspace experimentation, PROTOCOL notes, and as a cleanup
> exercise on the `develop` branch.

# Xbox/Xbox360 USB Gamepad Driver for Userspace

Xboxdrv is a userspace Xbox / Xbox 360 gamepad driver for Linux. It is an
alternative to the `xpad` kernel driver and supports Xbox classic pads,
Xbox 360 USB and wireless receivers, plus a number of third-party and
related devices (see `src/xpad_device.cpp`). Optional backends include
PlayStation 3 (USB), Wiimote (when built with CWiid), and generic USB.

Basic support for the Xbox 360 Chatpad on **wired** USB controllers is
included; Chatpad on wireless receivers is not supported. The headset is
not supported beyond optional raw dumps.

Use this driver when you need more configurability than `xpad`, or when
`xpad` does not work for a particular device. For most games, the kernel
driver is preferable.

## Source

* GitHub: https://github.com/xboxdrv/xboxdrv
* Default development branch: `develop` (cleanup target)
* Production / packaging reference: `stable`

## Requirements

### Build

* C++23 compiler (GCC or Clang)
* CMake ≥ 3.14
* pkg-config
* libusb-1.0
* libudev
* libevdev
* X11 (libX11)
* dbus-glib-1 / GLib
* Python 3 (D-Bus glue and `bin2h` generation)
* GTK 3 development files (still required by the current CMake
  configuration; not linked into the binary)

Optional:

* libcwiid — Wiimote support (`HAVE_CWIID` when detected)

Vendored C++ helpers live under `external/` (argpp, logmich, strutcpp,
tinycmmc, uinpp, unsebu, yaini) and are built via CMake
`add_subdirectory`; they are not separate system packages.

### Runtime

* `uinput` kernel module
* For daemon mode with a system bus name: D-Bus policy from
  `data/org.seul.Xboxdrv.conf`

### Nix

A flake is provided (`flake.nix`). Example:

```bash
nix build
```

## Compilation

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Development builds with a stricter warning set can use your usual
CMake/toolchain flags; the project sets `CXX_STANDARD 23`.

Example package set on Debian/Ubuntu-style systems:

```bash
sudo apt-get install \
  g++ cmake pkg-config \
  libusb-1.0-0-dev libudev-dev libevdev-dev \
  libx11-dev libdbus-glib-1-dev libglib2.0-dev \
  libgtk-3-dev python3
# optional:
# sudo apt-get install libcwiid-dev
```

Load uinput:

```bash
sudo modprobe uinput
```

To load it on boot, add `uinput` to `/etc/modules` (or the equivalent for
your distribution).

## Installation

```bash
cmake --install build
```

Prefix and DESTDIR work as usual:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
cmake --build .
DESTDIR=/tmp/stage cmake --install .
```

Running from the build tree without installing is fine for testing.

### Daemon / D-Bus policy

If you run xboxdrv as a daemon that owns `org.seul.Xboxdrv` on the system
bus, install the policy file:

```bash
sudo cp data/org.seul.Xboxdrv.conf /etc/dbus-1/system.d/
```

Without it, startup may fail with a D-Bus security policy error.

## Running

Detailed options are documented in the man page:

```bash
man -l doc/xboxdrv.1
```

Example configs live in `examples/`. USB protocol notes are in
`PROTOCOL`.

## Versioning

The top-level `VERSION` file is the single source of truth (e.g.
`0.9.0-dev` on `develop`). CMake and the Nix flake derive display and
package versions from it; see `AGENTS.md` for the full scheme.

## Status and branches

* **`stable`** — packaging-oriented line; leave it alone unless fixing
  regressions there.
* **`develop`** — cleanup and modernization target; goal is for `develop`
  to fully replace `stable`.

See `TODO.md` for the current parity and cleanup roadmap.

## License

GPL-3.0-or-later. See `COPYING`.
