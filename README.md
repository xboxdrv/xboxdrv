> [!WARNING]
> **xboxdrv is mostly obsolete for everyday use.** Prefer the in-kernel
> `xpad` driver and/or Steam Input. This tree remains useful for heavy
> remapping, userspace experimentation, and PROTOCOL notes.

# Xbox/Xbox360 USB Gamepad Driver for Userspace

Xboxdrv is a userspace Xbox / Xbox 360 gamepad driver for Linux. It is an
alternative to the `xpad` kernel driver and supports Xbox classic pads,
Xbox 360 USB and wireless receivers, plus a number of third-party and
related devices (see `src/xpad_device.cpp`). Optional backends include
PlayStation 3 (USB), Wiimote (when built with CWiid), generic USB, and an
experimental Steam Controller backend.

Chatpad support: **wired** USB controllers (interface 2) and experimental
**wireless** receiver multiplexing (`--chatpad`). Headset audio works on
**wired** Xbox 360 USB; **wireless** receiver headset support is experimental
(`--headset` / `--headset-pipewire` / `--headset-pulse` on the same slot).

Use this driver when you need more configurability than `xpad`, or when
`xpad` does not work for a particular device. For most games, the kernel
driver is preferable.

## Source

* GitHub: https://github.com/xboxdrv/xboxdrv
* Default branch: `master`
* Changelog: [`NEWS.md`](NEWS.md)

## Requirements

### Build

* C++23 compiler (GCC ≥ 13 or recent Clang)
* CMake ≥ 3.14
* pkg-config
* libusb-1.0
* libudev
* libevdev
* X11 (libX11) — only for optional `--help-x11keysym` key names
* dbus-glib-1 / GLib (includes `dbus-binding-tool` for glue generation)
* Python 3 (D-Bus glue and `bin2h` generation)

Optional:

* libcwiid — Wiimote support (`HAVE_CWIID` when detected)

Vendored C++ helpers live under `external/` and are built via CMake; they
are not separate system packages. No GTK or Boost dependency is required.

### Runtime

* `uinput` kernel module
* For daemon mode with a system bus name: D-Bus policy from
  `data/org.seul.Xboxdrv.conf`

### Nix

A flake is provided (`flake.nix`):

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

Example package set on Debian/Ubuntu-style systems:

```bash
sudo apt-get install \
  g++ cmake pkg-config \
  libusb-1.0-0-dev libudev-dev libevdev-dev \
  libx11-dev libdbus-glib-1-dev libglib2.0-dev \
  python3
# optional Wiimote:
# sudo apt-get install libcwiid-dev libbluetooth-dev
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

## License

GPL-3.0-or-later. See `COPYING`.
