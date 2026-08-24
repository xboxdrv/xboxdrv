# AGENTS.md — xboxdrv

Project rules for humans and agents working on this tree.

## Status and branches

- **`stable`**: working and good. Do not destabilise it.
- **`develop`**: the cleanup target. Goal is to bring `develop` to a state
  that can **replace `stable`**.
- xboxdrv is largely obsolete for most users (prefer kernel xpad + Steam
  Input). The tree is still useful for configurability, PROTOCOL notes, and
  as a cleaning exercise.

Work on `develop` unless explicitly told otherwise.

## Bundle / commit conventions (mandatory)

- Code is handed over as `git bundle`.
- Bundles must stack cleanly; each stacks on top of the previous one.
- Continuously numbered: `xboxdrv-001-…`, `xboxdrv-002-…`, …
- Numbers never repeat, even after resets.
- Bundles must use `HEAD` as ref.
- Small, task-focused commits.
- Author for all commits: `Ingo Ruhnke <grumbel@gmail.com>`
- Trailer on every commit: `Co-authored-by: Grok <grok@x.ai>`
- Prefer shallow checkouts; work in larger batches (multiple commits per
  batch) because `git clone` is slow.
- Checkout to a temporary directory, then `rsync` into artifacts (writing
  to artifacts is slow/expensive/unreliable; reading is fine). Multiple
  tries may be needed.
- nix store is read-only; adjust permissions after copying from it.
- Double-check all Nix quoting/escaping.
- If complexity grows too large, refactor.
- Never do quick hacks; step back and choose a clean solution.

## Version number handling

`VERSION` in the top-level directory is the **only source of truth**.
Do not hardcode the version elsewhere; derive it from this file.

### Rules

- Do **not** use `.gitattributes` `export-subst` (or similar) for `VERSION`.
  Keep it a plain text file (e.g. `0.8.8-dev`).
- **In git (development):** `VERSION` always carries a `-dev` suffix,
  e.g. `0.8.8-dev`.
- **Development builds** (when `VERSION` contains `-dev`) append the git
  revision count and short hash:
  - `0.8.8-dev.1615+gf1fb306`
  - `0.8.8-dev.1615+gf1fb306-dirty` (working tree dirty)
- **Release builds** (when `VERSION` has no `-dev`): use the file contents
  as-is, e.g. `0.8.8`. Do **not** append revision count or hash.
- **Revision count** (`revCount`): number of commits reachable from the
  current revision. It orders development builds. In Nix flakes this is
  `self.revCount` when available (see caveats below).
- **Short hash:** Nix `self.shortRev` or, if dirty, `self.dirtyShortRev`
  (already includes a `-dirty` suffix when applicable). Fallback: `"dirty"`.

### Format (development)

```text
{base}-dev.{revCount}+g{shortRev}
```

Roughly semver-compatible: `0.8.8-dev.1615` is a pre-release;
`+gf1fb306` is build metadata.

### Release process

1. Set `VERSION` to the release number **without** `-dev` (e.g. `0.8.8`).
2. Commit that change.
3. Tag the tree as `v` + that number (e.g. `v0.8.8`). The tag must match
   `VERSION` with a `v` prefix.
4. After the release, bump `VERSION` to the next `-dev` (e.g. `0.8.9-dev`)
   on the main/`develop` branch.

### CMake

- Read `VERSION` into `PROJECT_VERSION_FULL` unless packaging overrides it
  (`-DPROJECT_VERSION_FULL=...`).
- Nix should pass the **full** version (with `.revCount+g…` when applicable)
  via that flag so the build matches the package.
- `project(... VERSION ...)` only accepts a numeric
  `major[.minor[.patch[.tweak]]]`. Strip any `-dev` / `+…` suffix for
  CMake’s `PROJECT_VERSION`.
- Use the **numeric** `PROJECT_VERSION` for
  `write_basic_package_version_file` and similar.
- Keep `PROJECT_VERSION_FULL` for display / compile definitions.
- Expose the full version via `--version`.
- Prefer defining e.g. `PACKAGE_VERSION` / `XBOXDRV_VERSION` from
  `PROJECT_VERSION_FULL`.
- Do not hardcode `project(xboxdrv VERSION 0.x.y)`; always derive from
  `VERSION` / `PROJECT_VERSION_FULL`.

Example:

```cmake
cmake_minimum_required(VERSION 3.10...4.1)

# Source of truth: top-level VERSION (e.g. "0.8.8-dev"), or
# -DPROJECT_VERSION_FULL=... from packaging (Nix may append .<revCount>+g<rev>).
if(NOT DEFINED PROJECT_VERSION_FULL)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION_FULL LIMIT_COUNT 1)
endif()

# project() needs a numeric version; strip -dev / +git / similar suffixes.
string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" _project_version_cmake "${PROJECT_VERSION_FULL}")
project(xboxdrv VERSION ${_project_version_cmake} LANGUAGES CXX)

# target_compile_definitions(libxboxdrv PUBLIC PACKAGE_VERSION="${PROJECT_VERSION_FULL}")
```

### Nix flake

- Read the base from `VERSION`.
- If the base contains `-dev`, append `.{revCount}+g{shortRev}`.
- **`self.revCount` is not always present** (shallow clones, some path/dirty
  evaluations). Always use a fallback: `self.revCount or 0`.
- Coerce with `toString` when interpolating.
- Pass the result into the package and into CMake as
  `-DPROJECT_VERSION_FULL=${version}`.

Example:

```nix
versionBase = nixpkgs.lib.strings.removeSuffix "\n" (builtins.readFile ./VERSION);
gitRev = "${self.shortRev or self.dirtyShortRev or "dirty"}";
isDev = nixpkgs.lib.strings.hasInfix "-dev" versionBase;
version =
  if isDev then
    "${versionBase}.${toString (self.revCount or 0)}+g${gitRev}"
  else
    versionBase;
```

Use `version` as the derivation `version` and pass it through to CMake.

**Do not** always append git metadata: that would tag releases with git
metadata and can fail when `revCount` is missing.

### Caveats

| Topic | Detail |
|--------|--------|
| **Missing `revCount`** | Use `self.revCount or 0`. Otherwise evaluation can fail with `attribute 'revCount' missing`. |
| **Shallow clones** | `revCount` may be missing or not match a full-history count; uniqueness still comes mostly from the hash. |
| **Dirty trees** | Prefer `dirtyShortRev` when set; it typically already ends in `-dirty`. |
| **CMake `project(VERSION)`** | Never pass the full `0.8.8-dev.N+g…` string; only the leading numeric part. |
| **Package version files** | Prefer numeric `PROJECT_VERSION` so `find_package` version checks stay sane. |
| **Duplication** | Avoid a second hardcoded version in `project()`, the flake, or headers; generate from `VERSION`. |
| **Release vs tag** | Tag `v0.8.8` must match committed `VERSION` `0.8.8` (plus the `v` prefix only on the tag). |

### Checklist (application)

- [ ] Plain top-level `VERSION` (`x.y.z-dev` on `develop`)
- [ ] CMake reads `VERSION` → `PROJECT_VERSION_FULL`, numeric for `project()`
- [ ] Flake builds `version` with conditional `.revCount+g…` and `revCount or 0`
- [ ] `-DPROJECT_VERSION_FULL=${version}` from Nix
- [ ] `--version` prints `PROJECT_VERSION_FULL`

## Build notes

- Primary build: CMake. C++20 is already required (`CXX_STANDARD 20`).
- Nix flake exists; later work may convert most flake inputs into vendored
  `external/` git subtrees (can wait).
- `external/` already has git submodules (argpp, logmich, strutcpp, tinycmmc,
  uinpp, unsebu, yaini) **and** the flake pulls the same projects as flake
  inputs. Keep that dual story in mind; do not invent a third.
- Required at build time today: libusb-1.0, udev, fmt, X11, dbus-glib, GTK3,
  Python (dbus glue / bin2h). Optional: CWiid.
- Runtime: uinput kernel module.
- Do not add Boost; README still mentions it, CMake does not.

## Cleanup priorities (see TODO.md)

1. Versioning: drop `.gitattributes` `export-subst`; implement the VERSION
   scheme above in CMake + flake.
2. `src/symbols/`: unfinished parallel naming system; likely replace or
   delete in favour of a smaller table (see TODO.md).
3. Unfinished Chatpad (USB only, leaky GLib timeouts, wireless unsupported).
4. `{fmt}` → `std::format()` (C++20 is already on).
5. Bring `develop` to a state that can replace `stable`.

## Working practices

- Prefer small, focused commits that each leave the tree conceptually
  consistent.
- After structural change, re-check version logic under both plain CMake
  and the flake.
- When touching event names / config language, preserve public behaviour
  or document a deliberate break.
- The historical `TODO` file is a dump of years of notes; `TODO.md` is the
  current cleanup roadmap. Do not dump new work only into the old `TODO`.
