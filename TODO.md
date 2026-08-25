# TODO.md — xboxdrv (`develop`)

Cleanup roadmap and analysis of the current tree. Historical notes remain
in `TODO` (no suffix); this file is the working list.

**Analysed revision:** `04e87e64f0b5afa26c43a330e58604aee4f29b02`
(`Document German Chatpad layout; Shift LED follows held state`) on
**`develop`**.

**Branches**

- `stable`: working and good — leave it alone.
- `develop`: cleanup target. Goal = `develop` can replace `stable`.

**Merge-base (fork point):** `27cdd9c6a994f3059b8ae683adb711169341ffa5`
(2012-12-19, “Added additional bookkeeping to USBController…”).

- `develop` has ~548 commits not in `stable`.
- `stable` has ~120 commits not in `develop` (mostly post-2012 device IDs,
  bugfixes, and later CMake / Python3 / flake work on the stable line).

---

## What this project is

Userspace Xbox / Xbox 360 (plus some third-party, PS3, Wiimote, …) driver
for Linux. Reads via libusb or evdev, runs a filter / modifier / event
pipeline, emits via uinput. Daemon + D-Bus exist. Marked discontinued;
kernel xpad + Steam Input are preferred for most users.

**Layout (high level)**

| Path | Role |
|------|------|
| `src/` | C++ core |
| `src/controller/` | Device-specific USB/evdev/Wiimote backends |
| `src/axisevent/` `axisfilter/` `buttonevent/` `buttonfilter/` `modifier/` | Pipeline |
| `src/util/` | string/math/exec helpers |
| `examples/` | `.xboxdrv` configs + macros — still valuable |
| `doc/` | manpage sources, plots |
| `data/` | D-Bus policy, virtualkeyboard assets |
| `external/` | vendored git subtrees (argpp, logmich, strutcpp, tinycmmc, uinpp, unsebu, yaini); see REVISIONS |
| `PROTOCOL` | USB protocol notes — keep |
| `TODO` | years of accumulated notes — triage later |
| `flake.nix` | Nix flake (helpers vendored under `external/`) |

CMake requires modern C++ (C++23 intended for `std::format` / `std::print`).
README on `develop` is still stale (Boost, Gtk+2, Ubuntu 15.04, old homepage).
`stable` README has an obsolescence warning and updated CMake-centric build
notes.

---

## Investigation: `develop` vs `stable` (feature parity)

Goal: `develop` must completely replace `stable`. Investigation focuses on
code, not outdated docs. Many practical fixes that landed on `stable` after
the 2012 fork were re-implemented independently on `develop` during the
architecture rewrite; some were not.

### Divergent history (unusual / notable)

- Fork is very old (2012). `develop` was a long-running rewrite (threads →
  GLib main loop, ControllerMessage / ports, symbols work, C++ modernisation,
  Chatpad async, USB teardown hardening, subtree vendoring). Intermediate
  states on `develop` were known-broken (see historical log messages about
  “most controllers still broken”).
- `stable` continued to receive community device IDs, mapping fixes, and
  build-system updates (SCons → CMake, Python3, flake) with far less
  architectural change.
- Authors on `stable` after the fork: almost entirely Ingo Ruhnke, plus a
  handful of one-off contributors (device IDs, Saitek P3600, F710, PS4
  example, double-fork, cross-compile, typo fixes).
- No evidence of force-push weirdness needed for parity analysis; the
  important signal is the set of functional commits only on `stable`.

### Missing on `develop` (must port or re-implement)

| Item | Evidence | Notes |
|------|----------|--------|
| **Saitek P3600 controller** | ~~missing~~ | **Ported** — `src/controller/saitek_p3600_controller.{cpp,hpp}`, factory, enum, VID/PID `0x06a3:0xf51a`. Report layout and trigger/stick mapping preserved from stable (no debug printf). |

### Stable fixes already present (rewritten) on `develop`

Checked by content, not by shared SHA (SHAs differ):

- `--mimic-xpad` / `--mimic-xpad-wireless` BACK → `BTN_SELECT` (not `BTN_BACK`).
- `FourWayRestrictorModifier` uses configured axes (AbsPort / float), not
  hard-coded X1/Y1 then X2/Y2.
- USB disconnect / `LIBUSB_TRANSFER_NO_DEVICE` / cancel drain / idempotent
  `send_disconnect` (recent Chatpad + USB hardening commits go further than
  stable’s 2015 fixes).
- Exec button double-fork + `waitpid` (zombie avoidance).
- Daemon `set_device_usbids` for virtual devices.
- INI `\r` treated as whitespace (now in vendored `external/yaini`).
- Sensitivity spelling, regex escape for xboxdrvctl, many device ID rows
  (parallel history on `develop`).

### CLI / public config surface

- `--ui-buttonmap` / `--ui-axismap` still registered (map to internal
  KEYMAP / ABSMAP); INI sections keep backward-compat aliases.
- Help lists: `help-abs` / `help-key` / … (stable used similar names).
- Develop-only options of note: Wiimote-related, `evdev-relmap`, etc.
- No missing public option names identified that would break typical
  `stable` configs, aside from behaviour of missing device backends.

### Still incomplete / broken / risky on `develop` (code-level)

From FIXMEs, recent commits, and parity goals:

1. **Chatpad**
   - Wired USB path has been actively fixed (claim interface 2, keep-alive
     STALL noise, modifiers/backspace via keyboard device, German layout
     docs, Shift LED follows held state).
   - CAPS sticky (Orange+Shift) still “not implemented”.
   - Wireless Chatpad remains out of scope (same as stable).
   - Decide for the milestone: default on vs flag vs leave optional.

2. **Saitek P3600** — ported from stable.

3. **Daemon / multi-controller / hotplug**
   - Multiple FIXMEs in `xboxdrv_daemon.cpp` (thread cleanup, libusb ref,
     sleep hacks, “dirty hack” comments).
   - udev “devices twice” and “newer libudev only” comments remain.
   - Multi-controller UInput update / threading still listed as open.

4. **Force feedback / LED**
   - Code paths exist (rumble axis handler, per-controller `set_rumble` /
     `set_led`, skip after disconnect).
   - PS3 rumble “254 isn’t quite right / right motor on/off only”.
   - Xbox One wireless: serial/battery/LED mapping still FIXME.
   - Need explicit behavioural comparison vs stable for common pads
     (wired 360, wireless 360 receiver).

5. **Wiimote**
   - Thread vs main-loop safety, calibration hack, “valid in size” encoding
     still FIXME.
   - Optional CWiid dependency.

6. **Evdev controller**
   - Several “not implemented” stubs; extra event types ignored with FIXME.

7. **Build / packaging drift**
   - `VERSION` is `0.9.0-dev` on develop vs `0.8.8` on stable (intentional
     for the cleanup line).
   - ~~Flake/fmt inconsistency~~ resolved: logmich + unsebu on `<format>`;
     flake no longer pulls `fmt`.
   - README on develop still advertises Boost, Gtk+2, old homepage; stable
     README is closer to current reality (CMake, obsolescence warning).
   - Manpage / examples need a pass against actual option names and
     behaviour.

8. **Historical `TODO` file**
   - Still a large untriaged dump; actionable items should move here
     incrementally.

### Features develop has that stable does not (keep)

- Hardened USB teardown / disconnect (GitHub #239 class of bugs).
- Substantial Chatpad USB improvements.
- C++ modernisation, vendored `external/` subtrees, libevdev-based name
  tables, yaini, uinpp/unsebu layout, CMake + flake as primary build.
- Symbols / naming cleanup (old parallel `src/symbols/` system removed;
  public evdev/X11 language retained).
- Xbox One wireless controller backend entries and related rows.

### Recommended parity work order

1. ~~Port Saitek P3600~~ (done).
2. ~~Resolve fmt vs std::format~~ (done).
3. **README + manpage** aligned with develop (obsolescence note, deps,
   build, option names).
4. **Daemon / hotplug / multi-slot** smoke-test vs stable behaviour; close
   or ticket the remaining FIXMEs that affect replaceability.
5. **FF / LED** smoke-test on wired 360 + wireless receiver.
6. Triage historical `TODO` into this file or discard.
7. Confirm `develop` builds (plain CMake and flake) and runs at least as
   well as `stable` for the device set both claim to support.

---

## Findings (structural — carried forward)

### 1. Two naming systems (the `src/symbols/` mess) — largely done

Old public config language (`evdev_helper` + libevdev) kept; unfinished
parallel YAML/Ruby `src/symbols/` system deleted in earlier work. Follow-up:
small test that important names still resolve.

### 2. Chatpad / virtual keyboard / D-Bus — in progress

See parity section. Virtual keyboard still optional/quarantine candidate.
D-Bus still effectively required for daemon path.

### 3. Versioning scheme

See AGENTS.md. `VERSION` is source of truth (`0.9.0-dev` on develop).
CMake derives numeric `project(VERSION …)`; flake appends
`.revCount+g…` when `-dev`. Keep that contract.

---

## Phases (status)

### Phase 1 — symbols / naming

- [x] Replace unfinished symbols system; keep public evdev/X11 language.
- [x] Delete `src/symbols/`.
- [x] Fix Wiimote nunchuk accel copy-paste (historical).
- [ ] Small test that important names still resolve.

### Phase 1b — libevdev for uinput names

- [x] Replace hand-rolled tables with libevdev.
- [x] Keep `KEY_#N` / `JS_N` / X11 `XK_*` special cases.
- [ ] Drop X11 keysym path later if unused.

### Phase 2 — Chatpad and dead weight

- [x] Own GLib timeouts in `Chatpad` destructor.
- [x] Wired Chatpad claim interface + keep-alive / modifier fixes (ongoing polish).
- [ ] CAPS sticky (Orange+Shift) still unimplemented.
- [ ] Decide: USB Chatpad on by default vs flag vs remove for the milestone.
- [ ] Quarantine or drop virtual keyboard if not wired up.
- [ ] Make D-Bus optional if still hard dependency for non-daemon path.

### Phase 3 — `std::format`

- [x] Replace `fmt::` in main tree and vendored logmich/unsebu with
      `std::format` / `std::vformat` / `<format>`.
- [x] Drop `fmt` from flake `buildInputs` and from logmich/unsebu CMake.
- [x] Fix missing `{}` in two unsebu `libusb_submit_transfer` error strings.
- [x] Fix `%04x` leftover in `usb_controller.cpp` (historical note).

### Phase 4 — Stability vs `stable` (parity)

- [x] USB disconnect/teardown hardening.
- [x] Wired Chatpad claim + soft-fail paths.
- [x] **Port Saitek P3600 controller + VID/PID from stable.**
- [ ] Triage historical `TODO`; move remaining actionable items here.
- [ ] Multi-controller UInput update / threading.
- [ ] Daemon hotplug, basic FF, LED — behavioural check vs stable.
- [ ] Drop log noise / temporary debug helpers.
- [ ] README + manpage pass (develop is stale; stable is closer).
- [ ] Confirm `develop` builds and behaves at least as well as `stable`
      for supported controllers.

### Phase 5 — Later

- [x] Vendor selected flake dependencies into `external/` subtrees.
- [ ] Drop ancient Travis if unused; refresh GitLab CI.
- [ ] Further architecture (In/Out ports, half-axis) only after the above.

---

## Notes for later agents

- Clean solution over a quick hack.
- One logical change per commit where practical.
- Public config compatibility: preserve or document the break.
- Prefer evidence from code and `git log` over README/TODO claims.
- Next concrete code step for parity: **README + manpage** pass.
