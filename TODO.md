# TODO.md — xboxdrv (`master`)

Cleanup roadmap and analysis of the current tree. Historical notes remain
in `TODO` (no suffix); this file is the working list.

**Analysed revision:** `04e87e64f0b5afa26c43a330e58604aee4f29b02`
(`Document German Chatpad layout; Shift LED follows held state`) on
**`master`**.

**Branches**

- `0.8.x` (formerly `stable`): maintenance line for the 0.8 series.
- `master` (formerly `develop`): active development; default branch.

**Merge-base (fork point):** `27cdd9c6a994f3059b8ae683adb711169341ffa5`
(2012-12-19, “Added additional bookkeeping to USBController…”).

- `master` has ~548 commits not in `stable`.
- `stable` has ~120 commits not in `master` (mostly post-2012 device IDs,
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
| `doc/` | manpage sources (`xboxdrv.xml`), plots, design notes (e.g. chatpad layout) |
| `data/` | D-Bus policy, virtualkeyboard assets |
| `external/` | vendored git subtrees (argpp, logmich, strutcpp, tinycmmc, uinpp, unsebu, yaini); see REVISIONS |
| `PROTOCOL` | USB protocol notes — keep |
| `TODO` | years of accumulated notes — triage later |
| `flake.nix` | Nix flake (helpers vendored under `external/`) |

CMake requires C++23 (`std::format` / `std::print`). README on `master` is
aligned with the current build (CMake, libevdev, no Boost/{fmt}).

---

## Investigation: `master` vs `stable` (feature parity)

Goal: `master` must completely replace `stable`. Investigation focuses on
code, not outdated docs. Many practical fixes that landed on `stable` after
the 2012 fork were re-implemented independently on `master` during the
architecture rewrite; some were not.

### Divergent history (unusual / notable)

- Fork is very old (2012). `master` was a long-running rewrite (threads →
  GLib main loop, ControllerMessage / ports, symbols work, C++ modernisation,
  Chatpad async, USB teardown hardening, subtree vendoring). Intermediate
  states on `master` were known-broken (see historical log messages about
  “most controllers still broken”).
- `stable` continued to receive community device IDs, mapping fixes, and
  build-system updates (SCons → CMake, Python3, flake) with far less
  architectural change.
- Authors on `stable` after the fork: almost entirely Ingo Ruhnke, plus a
  handful of one-off contributors (device IDs, Saitek P3600, F710, PS4
  example, double-fork, cross-compile, typo fixes).
- No evidence of force-push weirdness needed for parity analysis; the
  important signal is the set of functional commits only on `stable`.

### Missing on `master` (must port or re-implement)

| Item | Evidence | Notes |
|------|----------|--------|
| **Saitek P3600 controller** | ~~missing~~ | **Ported** — `src/controller/saitek_p3600_controller.{cpp,hpp}`, factory, enum, VID/PID `0x06a3:0xf51a`. Report layout and trigger/stick mapping preserved from stable (no debug printf). |

### Stable fixes already present (rewritten) on `master`

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
  (parallel history on `master`).

### CLI / public config surface

- `--ui-buttonmap` / `--ui-axismap` still registered (map to internal
  KEYMAP / ABSMAP); INI sections keep backward-compat aliases.
- Help lists: `help-abs` / `help-key` / … (stable used similar names).
- Develop-only options of note: Wiimote-related, `evdev-relmap`, etc.
- No missing public option names identified that would break typical
  `stable` configs, aside from behaviour of missing device backends.

### Still incomplete / broken / risky on `master` (code-level)

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
   - ~~libusb device leak on hotplug~~ fixed via `libusb_ref_device` /
     `unref` in `USBController` (shared wireless receiver safe).
   - ~~stale cleanup_threads FIXME~~ → `on_controller_disconnect()` on match.
   - udev monitor log noise reduced; double-match race documented.
   - Multi-controller UInput update / threading still open.

4. **Force feedback / LED**
   - Wiring intact: uinpp FF callback → slot `set_rumble` → controller.
   - Skip after disconnect already present.
   - PS3 rumble quantisation and Xbox One wireless LED/battery still FIXME.
   - Hardware smoke vs stable still recommended when a pad is available.

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
   - ~~Stale README~~ updated (obsolescence warning, current deps, cmake
     build, repo URLs). Manpage issue URL pointed at xboxdrv/xboxdrv;
     full manpage content review still optional later.

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
3. ~~README + manpage~~ (README done; manpage issue URL fixed).
4. **Daemon / hotplug / multi-slot** smoke-test vs stable behaviour; close
   or ticket the remaining FIXMEs that affect replaceability.
5. **FF / LED** smoke-test on wired 360 + wireless receiver.
6. Triage historical `TODO` into this file or discard.
7. Confirm `master` builds (plain CMake and flake) and runs at least as
   well as `stable` for the device set both claim to support.

---

## Findings (structural — carried forward)

### 1. Two naming systems (the `src/symbols/` mess) — largely done

Old public config language (`evdev_helper` + libevdev) kept; unfinished
parallel YAML/Ruby `src/symbols/` system deleted in earlier work. Follow-up:
small test that important names still resolve.

### 2. Chatpad / virtual keyboard / D-Bus — in progress

See parity section. Virtual keyboard still optional/quarantine candidate.
D-Bus is daemon-only today (`--daemon`); single-controller mode has no bus
export. D-Bus now uses GDBus (Gio); dbus-glib removed. See “Path to improvements”
below for a concrete migration sketch.

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
- [x] Make D-Bus optional at build/runtime for non-daemon builds (see Path to improvements).

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
- [ ] Multi-controller UInput update / threading (still open).
- [x] Daemon hotplug: process_match runs on_controller_disconnect; USBController
      libusb_ref/unref for shared devices; quieter udev monitor logs.
- [x] FF/LED code paths reviewed (uinpp callback → ControllerSlotConfig::set_rumble
      → Controller::set_rumble); hardware smoke still recommended.
- [x] Replace remaining live `log_tmp` with `log_debug` (wiimote/main).
- [x] README + manpage pass (obsolescence note, deps, build, repo URLs).
- [x] Drop unused GTK CMake requirement; define/link HAVE_CWIID when cwiid found.
- [x] Confirm `master` builds (user-verified).
- [ ] Behavioural parity vs `stable` for supported controllers (hardware).

### Phase 5 — Later

- [x] Vendor selected flake dependencies into `external/` subtrees.
- [x] Drop ancient Travis; refresh GitLab CI (Ubuntu 24.04, no fmt/gtk).
- [ ] Further architecture (In/Out ports, half-axis) only after the above.

---

### Headset (USB + wireless)

- [x] Wired Xbox 360 USB: G.726 mic/phone, `--headset-pulse` / `--headset-pipewire`, mic gain.
- [x] Generalise `Headset` for configurable interface / endpoints (wired defaults unchanged).
- [x] Wire `--headset*` into `Xbox360WirelessController` (slot N → IF 2N+1, EP 2N+2).
- [x] **Hardware smoke wireless headset**: mic + phone work with wired-style 32-byte
      G.726 framing; user reports cleaner audio than wired USB headset path.
- [ ] Wireless-only headset (no pad) / battery status from PROTOCOL still unused.
- [x] README + man page headset section refreshed (wired + wireless).
- [x] Soft-fail wireless headset interface claim (pad keeps working).

## Path to improvements

Actionable follow-ups from recent work (startup UX, filters, headset, docs).
Prefer small commits; keep public CLI stable or document breaks.

### Documentation layout

- [x] Unify `docs/` into `doc/` (chatpad layout note lives next to the manpage).
- [ ] Keep manpage (`doc/xboxdrv.xml` → `doc/xboxdrv.1`) as the user-facing
      reference; avoid a second parallel doc tree.
- [ ] Optional: short `doc/README.md` index (manpage, PROTOCOL, chatpad layout).

### Startup / UX

- [x] Feature-status block (uinput, force-feedback, chatpad, headset, detach).
- [x] Real `/dev/input` nodes via `UI_GET_SYSNAME` + sysfs (not guessed js/event indexes).
- [x] Default LED from controller slot, not guessed `jsN` (issue #168 class of bugs).
- [ ] Feature-status / device list in **daemon** mode (parity with single-controller).
- [ ] Group `--help` by feature (headset / chatpad / FF) if argpp allows without noise.

### Force feedback / LED / test-rumble

- [x] Restore `--test-rumble` (LT/RT → motors; was `#if 0` after axis-port refactor).
- [x] `--deadzone` / calibration / sensitivity / relative maps rewired via `AxismapModifier::add_filter`.
- [ ] Hardware smoke: `--force-feedback` from games; `--test-rumble` on wired + wireless.
- [ ] **EV_LED from uinput**: not wired. Kernel `EV_LED`/`LED_MISC` is on/off only —
      **not** the Xbox 0–15 pattern byte. Options if pursued:
  1. Keep patterns on `--led` / D-Bus only (recommended default).
  2. Optional lossy map: `LED_MISC` on → player-1 solid, off → all off.
  3. Private non-standard map (document as xboxdrv-only; avoid claiming Linux standard).
- [ ] Do **not** claim `EV_LED` value carries xboxdrv `--led` codes.

### D-Bus

Current state (daemon only):

| Object | Interface | Methods |
|--------|-----------|---------|
| `/org/seul/Xboxdrv/Daemon` | `org.seul.Xboxdrv.Daemon` | `Status`, `Shutdown` |
| `/org/seul/Xboxdrv/ControllerSlots/N` | `org.seul.Xboxdrv.Controller` | `SetLed`, `SetRumble`, `SetConfig` |

Improvement path:

1. **Optional dependency** — [x] CMake `WITH_DBUS` (default ON) + `#ifdef HAVE_DBUS`; non-daemon builds skip
   dbus code when OFF (`--dbus disabled` already skips runtime export).
2. **Replace dbus-glib** — [x] migrated to GDBus (Gio);
   keep the same object paths and method names for script compatibility.
3. **Signals** — emit `ControllerConnected` / `ControllerDisconnected` (slot id)
   instead of clients polling `Status`.
4. **Properties** — expose LED, rumble, config index, battery (wireless) as
   readable properties where data exists.
5. **Finish XML stubs** — `reset_leds`, `disconnect SLOT`, rumble enable/gain
   only if still useful after properties.
6. **Policy** — keep `data/` system-bus policy in sync if system bus stays supported.
7. **Single-controller mode** — optional light export (same `Controller` iface on
   slot 0) so tools work without `--daemon`; low priority.

### Headset

- [x] Wired + wireless PipeWire/Pulse paths; mic gain; soft-fail wireless claim.
- [ ] Wireless-only headset (no pad) if hardware reports it.
- [ ] Battery high-nibble (charging / pack type) still undocumented; keep logging
      `raw & 0x03` as bar level until evidence improves.

### Code hygiene (from recent audit)

- [x] `DpadRestrictorModifier` reimplemented on key ports (was empty `#if 0`).
- [ ] Remaining `#if 0` / debug-only blocks: IR2Axis, button_map alternate merge,
      message_processor rumble-test is restored (done).
- [ ] Xbox One wireless: serial / battery / button-state FIXMEs.
- [ ] Prefer deleting dead `#if 0` once confirmed superseded.

---

## Notes for later agents

- Clean solution over a quick hack.
- One logical change per commit where practical.
- Public config compatibility: preserve or document the break.
- Prefer evidence from code and `git log` over README/TODO claims.
- Build confirmed (plain CMake with WARNINGS/WERROR path in CI).
- Next: hardware FF/LED/`--test-rumble` smoke; multi-controller UInput
  threading still open; D-Bus optional/migration path above; optional deeper
  historical TODO triage.
- [x] Sync missing kernel xpad XTYPE_XBOX360 IDs into xpad_device
  (Xbox One/Series left out until testable).
