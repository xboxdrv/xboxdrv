{
  description = "Xbox360 USB Gamepad Userspace Driver";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          xboxdrv = pkgs.stdenv.mkDerivation {
            pname = "xboxdrv";
            version = "0.9.0";
            src = nixpkgs.lib.cleanSource ./.;
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.pkg-config
            ];
            buildInputs = [
              pkgs.at-spi2-core
              pkgs.bluez
              pkgs.dbus-glib
              pkgs.epoxy
              pkgs.fmt
              pkgs.glib
              pkgs.gobject-introspection
              pkgs.gtest
              pkgs.gtk3
              pkgs.libdatrie
              pkgs.libselinux
              pkgs.libsepol
              pkgs.libthai
              pkgs.libudev
              pkgs.libusb1
              pkgs.libxkbcommon
              pkgs.pcre
              pkgs.python3
              pkgs.python3Packages.dbus-python
              pkgs.util-linux
              pkgs.xorg.libX11
              pkgs.xorg.libXdmcp
              pkgs.xorg.libXtst
            ];
          };
        };
        defaultPackage = packages.xboxdrv;
      });
}
