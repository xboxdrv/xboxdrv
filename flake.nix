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
      in rec {
        packages = rec {
          default = xboxdrv;

          xboxdrv = pkgs.stdenv.mkDerivation {
            pname = "xboxdrv";
            version = "0.8.8";

            src = ./.;

            enableParallelBuilding = true;

            installPhase = ''
              make install PREFIX=$out
            '';

            nativeBuildInputs = with pkgs; [
              pkg-config
              scons
            ];

            buildInputs = with pkgs; [
              dbus-glib
              glib
              libusb1
              xorg.libX11
            ];

            propagatedBuildInputs = with pkgs; [
              python3Packages.dbus-python
            ];
          };
        };

        apps = rec {
          default = xboxdrv;

          xboxdrv = flake-utils.lib.mkApp {
            drv = packages.xboxdrv;
            exePath = "/bin/xboxdrv";
          };

          xboxdrvctl = flake-utils.lib.mkApp {
            drv = packages.xboxdrv;
            exePath = "/bin/xboxdrvctl";
          };
        };
      }
    );
}
