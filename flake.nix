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
        # VERSION is the only source of truth (e.g. "0.9.0-dev").
        # Dev builds: 0.9.0-dev.<revCount>+g<shortRev>[-dirty]
        # Release builds: use VERSION as-is (no git metadata).
        versionBase = nixpkgs.lib.fileContents ./VERSION;
        # shortRev on clean trees; dirtyShortRev already includes "-dirty".
        gitRev = "${self.shortRev or self.dirtyShortRev or "dirty"}";
        isDev = nixpkgs.lib.strings.hasInfix "-dev" versionBase;
        version =
          if isDev then
            "${versionBase}.${toString (self.revCount or 0)}+g${gitRev}"
          else
            versionBase;

      in {
        packages = rec {
          default = xboxdrv;

          xboxdrv = pkgs.stdenv.mkDerivation {
            pname = "xboxdrv";
            version = version;
            src = nixpkgs.lib.cleanSource ./.;
            cmakeFlags = [
              "-DPROJECT_VERSION_FULL=${version}"
            ];
            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config
            ];
            buildInputs = with pkgs; [
              dbus-glib
              glib
              udev
              libusb1
              libevdev
              libx11
              pipewire
              (pkgs.python3.withPackages (p: [
                p.dbus-python
              ]))
              # Optional Wiimote support (HAVE_CWIID when present):
              # cwiid bluez
              # Vendored C++ helpers under external/ are built via
              # CMake add_subdirectory (see build_dependencies()).
              # Formatting uses C++20/23 <format> (no {fmt}).
            ];
          };
        };
      }
    );
}
