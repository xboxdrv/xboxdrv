{
  description = "Xbox360 USB Gamepad Userspace Driver";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;
        # VERSION is the only source of truth (e.g. "0.9.0-dev").
        # Dev builds: 0.9.0-dev.<revCount>+g<shortRev>[-dirty]
        # Release builds: use VERSION as-is (no git metadata).
        versionBase = lib.fileContents ./VERSION;
        # shortRev on clean trees; dirtyShortRev already includes "-dirty".
        gitRev = "${self.shortRev or self.dirtyShortRev or "dirty"}";
        isDev = lib.strings.hasInfix "-dev" versionBase;
        version =
          if isDev then
            "${versionBase}.${toString (self.revCount or 0)}+g${gitRev}"
          else
            versionBase;

        mkXboxdrv = { withDbus ? true }:
          pkgs.stdenv.mkDerivation {
            pname = "xboxdrv";
            inherit version;
            src = lib.cleanSource ./.;
            cmakeFlags = [
              "-DPROJECT_VERSION_FULL=${version}"
            ] ++ lib.optional (!withDbus) "-DWITH_DBUS=OFF";
            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config
            ];
            buildInputs = with pkgs; [
              glib
              udev
              libusb1
              libevdev
              libx11
              pipewire
              (python3.withPackages (p: [
                p.dbus-python
              ]))
              # Optional Wiimote support (HAVE_CWIID when present):
              # cwiid bluez
              # Vendored C++ helpers under external/ are built via
              # CMake add_subdirectory (see build_dependencies()).
              # Formatting uses C++20/23 <format> (no {fmt}).
            ];
          };

        xboxdrv = mkXboxdrv { };
        xboxdrv-no-dbus = mkXboxdrv { withDbus = false; };

      in {
        packages = {
          default = xboxdrv;
          inherit xboxdrv xboxdrv-no-dbus;
        };

        # `nix flake check` builds these. That is the useful signal:
        # default package, WITH_DBUS=OFF path, and a --version smoke test.
        checks = {
          build = xboxdrv;
          build-no-dbus = xboxdrv-no-dbus;

          version = pkgs.runCommand "xboxdrv-version-check" {
            nativeBuildInputs = [ xboxdrv ];
          } ''
            set -euo pipefail
            outv=$(xboxdrv --version | head -n1)
            echo "xboxdrv --version => $outv"
            # Dev builds embed VERSION base; release builds match exactly.
            echo "$outv" | grep -F -q ${lib.escapeShellArg versionBase}
            touch "$out"
          '';
        };
      }
    );
}
