# TODO.md — xboxdrv (`develop`)

Cleanup roadmap and analysis of the current tree. Historical notes remain
in `TODO` (no suffix); this file is the working list.

**Analysed revision:** `56ffb143e047bb38609c39c80a1f49d80d3fb3dd`
(`Fix regex issue in xboxdrvctl`) on **`develop`**.

**Branches**

- `stable`: working and good — leave it alone.
- `develop`: cleanup target. Goal = `develop` can replace `stable`.

---

## What this project is

Userspace Xbox / Xbox 360 (plus some third-party, PS3, Wiimote, …) driver
for Linux. Reads via libusb or evdev, runs a filter / modifier / event
pipeline, emits via uinput. Daemon + D-Bus exist. Marked discontinued;
kernel xpad + Steam Input are preferred for most users.

**Layout (high level)**

| Path | Role |
|------|------|
| `src/` | C++ core (~118 `.cpp` files) |
| `src/controller/` | Device-specific USB/evdev/Wiimote backends |
| `src/axisevent/` `axisfilter/` `buttonevent/` `buttonfilter/` `modifier/` | Pipeline |
| `src/symbols/` | Unfinished name tables (see below) |
| `src/util/` | string/math/exec helpers |
| `examples/` | `.xboxdrv` configs + macros — still valuable |
| `doc/` | manpage sources, plots |
| `data/` | D-Bus policy, virtualkeyboard assets |
| `external/` | git submodules (argpp, logmich, strutcpp, tinycmmc, uinpp, unsebu, yaini) |
| `PROTOCOL` | USB protocol notes — keep |
| `TODO` | years of accumulated notes — triage later |
| `flake.nix` + `.gitmodules` | **two** ways to get the same C++ helper libs |

CMake already requires **C++20**. README is stale (Boost, Gtk+2, Ubuntu 15.04,
old homepage).

---

## Findings

### 1. Two naming systems (the `src/symbols/` mess)

This is the biggest structural problem.

**Old, still used for the public config language**

- `src/evdev_helper.{hpp,cpp}` + `EnumBox`
- generated-looking `src/key_list.x`, `abs_list.x`, `rel_list.x`
  (from `src/gen_event_lists.rb`)
- Maps `KEY_*` / `BTN_*` / `ABS_*` / `REL_*` / X11 keysyms for
  `--ui-buttonmap`, `--ui-axismap`, evdev input, etc.

**New, unfinished, compiled in, only used internally**

- `src/symbols/*.yaml` (evdev dump ~1646 lines, plus `gamepad`, `xbox`,
  `classic`, `guitar`, `hama-crux`, `joystick`, `nunchuk`, `playstation`,
  `wiimote`)
- Ruby 1.8 generators: `gen_symbols.rb`, `gen_evdev_yaml.rb`
- Generated C++: `init_key.cpp` (~2763 lines), `init_abs.cpp`, `init_rel.cpp`
- Runtime: `Environment` / `Namespace` / `Symbol` / `Name<>` → `KeyName`,
  `AbsName`, `RelName`

Call sites of `KeyName` / `AbsName` today:

- `src/xbox360_default_names.cpp` (`xbox.a`, `xbox.start`, …)
- `src/controller/hama_crux_controller.cpp`
- `src/controller/wiimote_controller.cpp` (also has a copy-paste bug:
  nunchuk accel Y/Z use `nunchuk.acc_x` / `nunchuk.acc_y` twice)
- `src/modifier/compat_modifier.cpp` (`gamepad.dpad_*`)

`Name<>` **requires a dotted name**. Unqualified names throw
`"user variables not implemented yet"`. YAML `evdev:` fields are **not**
wired into the generated C++ (the generator only emits names, aliases, and
`provides`). So the “new” system does not actually bind to Linux event
codes.

The old `TODO` already records:

```text
SymbolTable<T>::get(): lookup failure for: 'gamepad.start'
```

(`xbox360_default_names` uses `xbox.start`; `gamepad.start` is a different
namespace.)

**Recommendation:** `src/symbols/` should go, or shrink to a small static
alias table. Do **not** keep YAML + Ruby + 3k-line generated init files.
Controller-local names (`xbox.a`, `hama-crux.crouch`) can be plain string
ids in `ControllerMessageDescriptor` without a global symbol environment.
Linux codes stay in `evdev_helper` (or a thin wrapper over `<linux/input.h>`).

### 2. Chatpad is unfinished

- USB Chatpad only (`src/chatpad.{cpp,hpp}`, `chatpad.xkb`). Wireless is
  explicitly unsupported.
- Init is a multi-state USB control-transfer machine (`kStateInit1` …).
- Known issues:
  - `FIMXE: must keep track of sources and destroy them in ~Chatpad()` —
    GLib `g_timeout_add` leak / use-after-free risk on destroy.
  - Historical: no `0x1b` when Chatpad is plugged after xboxdrv is already
    running.
  - `xbox360_controller.cpp`: `FIXME: maybe a proper indicator for the
    activity on the chatpad`.
- Headset is dump-only (`src/headset.cpp`).

For “replace `stable`”: either finish the timeout ownership and document
USB-only, or disable Chatpad behind a clear flag so it cannot crash the
driver. Do not expand wireless Chatpad in this milestone.

### 3. Versioning is the old `export-subst` scheme

- `VERSION` currently contains `$Format:%(describe)$`.
- `.gitattributes`: `/VERSION export-subst`.
- CMake: `include(GetProjectVersion)` from **tinycmmc** (submodule), then
  `PACKAGE_VERSION="${PROJECT_VERSION}"`.
- Flake: if `VERSION` does not start with `v`, it invents
  `${lastModifiedDate}-${shortRev}` and **rewrites** `VERSION` in
  `postPatch`. That is incompatible with the new rules in `AGENTS.md`.

**Do:** delete `.gitattributes` (or stop using it for version). Make
`VERSION` a plain `x.y.z-dev` file. Implement the CMake + flake scheme in
`AGENTS.md`.

### 4. `{fmt}` replaced with `std::format` / `std::print`

C++23 is required. `{fmt}` has been dropped from CMake and the flake.
Watch remaining printf-style leftovers if any new format strings are added
(`usb_controller.cpp` had `"%04x:%04x"` mixed with fmt).

### 5. Dual dependency story (`external/` vs flake inputs)

Git submodules **and** flake inputs both pull argpp, logmich, strutcpp,
tinycmmc, uinpp, unsebu, yaini. CMake `build_dependencies()` uses
`find_package` then `add_subdirectory(external/…)`.

Vendoring flake-only bits into `external/` subtrees can wait. When we do
it, pick **one** mechanism (submodules *or* subtrees), not a third.

Shallow clones without `git submodule update` will fail CMake unless the
flake-provided packages satisfy `find_package`.

### 6. Other mess (not blocking the first cleanups)

- CMake `file(GLOB … src/uinput/*.cpp)` — **that directory does not exist**.
- GTK3 is **required** (`pkg_search_module(GTK REQUIRED gtk+-3.0)`);
  virtual keyboard looks unused/unfinished. CWiid is optional but Wiimote
  code is compiled in.
- Tests: GTest block is commented out; `BUILD_TESTS` only builds old
  `test/*_test.cpp` binaries. `src/symbols/test.cpp` is a leftover main().
- `dbus-glib` is ancient; making D-Bus optional is still desirable.
- ~69 `FIXME` / `TODO` / `#if 0` / `implement me` hits under `src/`.
- README lists Boost; CMake does not use Boost.
- Daemon / multi-controller UInput update issues remain in the old `TODO`.
- `xboxdrvctl` is a Python script; last commit was a regex fix.

**Still valuable:** `PROTOCOL`, `examples/`, the filter/modifier/event
architecture, daemon + hotplug concept.

---

## Roadmap (ordered for “`develop` can replace `stable`”)

### Phase 0 — Housekeeping

- [x] This file + `AGENTS.md` (bundle `xboxdrv-001`).
- [x] Remove `.gitattributes` / `export-subst`.
- [x] Plain `VERSION` with `-dev` suffix; CMake + flake per `AGENTS.md`.
- [x] `--version` prints `PROJECT_VERSION_FULL` (`PACKAGE_VERSION` is defined from it).
- [ ] Sanity-check a plain CMake configure *and* a flake evaluation for
      the version string.

### Phase 1 — Symbols

- [ ] Map remaining `KeyName` / `AbsName` / `RelName` / `Environment` uses.
- [ ] Replace with a small static table or descriptor-local string ids.
- [ ] Keep the public evdev/X11 name language (`evdev_helper`) working.
- [ ] Delete YAML, Ruby generators, and generated `init_*.cpp`.
- [ ] Fix the Wiimote nunchuk accel copy-paste while touching that file.
- [ ] Small test that important names still resolve.

### Phase 2 — Chatpad and dead weight

- [ ] Own GLib timeouts in `Chatpad` destructor (or stop using them).
- [ ] Decide: USB Chatpad on by default vs flag vs remove for the
      milestone. Wireless stays out of scope.
- [ ] Quarantine or drop virtual keyboard if it is not wired up.
- [ ] Make D-Bus optional if it is still a hard dependency for the
      non-daemon path.

### Phase 3 — `std::format`

- [x] Replace `fmt::format` / `fmt::format_to` / `fmt::print` with
      `std::format` / `std::format_to` / `std::print`.
- [x] Drop `find_package(fmt)` and flake `fmt` input.
- [x] Fix the `%04x` leftover in `usb_controller.cpp`.

### Phase 4 — Stability vs `stable`

- [ ] Triage the historical `TODO`; move remaining actionable items here.
- [ ] Multi-controller UInput update / threading.
- [ ] Daemon hotplug, basic FF, LED, common examples.
- [ ] Drop log noise / temporary debug helpers.
- [ ] README + manpage pass.
- [ ] Confirm `develop` builds and behaves at least as well as `stable`
      for supported controllers.

### Phase 5 — Later

- [ ] Vendor selected flake dependencies into `external/` subtrees
      (single mechanism).
- [ ] Drop ancient Travis if unused; refresh GitLab CI.
- [ ] Further architecture (In/Out ports, half-axis) only after the above.

---

## Notes for later agents

- Clean solution over a quick hack.
- One logical change per commit where practical.
- Public config compatibility: preserve or document the break.
- Next bundle after this one: versioning (`.gitattributes` + `VERSION` +
  CMake/flake) is the natural first *code* change; symbols next.
