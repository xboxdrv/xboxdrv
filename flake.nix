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
        versionBase = nixpkgs.lib.fileContents ./VERSION;
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
              at-spi2-core
              bluez
              dbus-glib
              glib
              gobject-introspection
              gtest
              gtk3
              libdatrie
              libselinux
              libsepol
              libthai
              udev
              libusb1
              libevdev
              libxkbcommon
              pcre
              (pkgs.python3.withPackages (p: [
                p.dbus-python
              ]))
              util-linux
              libx11
              libxdmcp
              libxtst
              # Vendored C++ helpers under external/ are built via
              # CMake add_subdirectory (see build_dependencies()).
              # fmt is still required by some of those helpers.
              fmt
            ];
          };
        };
      }
    );
}
