{
  description = "Xbox360 USB Gamepad Userspace Driver";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
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
            version = "0.8.8";
            src = nixpkgs.lib.cleanSource ./.;
            enableParallelBuilding = true;
            nativeBuildInputs = [
              pkgs.scons
              pkgs.pkg-config
            ];
            installPhase = ''
              make install PREFIX=$out
            '';
            buildInputs = [
              pkgs.xorg.libX11
              pkgs.libusb1
              pkgs.boost
              pkgs.glib
              pkgs.dbus-glib
            ];
          };
        };
        defaultPackage = packages.xboxdrv;
      });
}
