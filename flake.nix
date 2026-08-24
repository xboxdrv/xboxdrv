{
  description = "Xbox360 USB Gamepad Userspace Driver";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    argpp.url = "github:grumbel/argpp/stable";
    argpp.inputs.nixpkgs.follows = "nixpkgs";
    argpp.inputs.flake-utils.follows = "flake-utils";
    argpp.inputs.tinycmmc.follows = "tinycmmc";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    strutcpp.url = "github:grumbel/strutcpp";
    strutcpp.inputs.nixpkgs.follows = "nixpkgs";
    strutcpp.inputs.tinycmmc.follows = "tinycmmc";

    logmich.url = "github:logmich/logmich";
    logmich.inputs.nixpkgs.follows = "nixpkgs";
    logmich.inputs.tinycmmc.follows = "tinycmmc";

    uinpp.url = "github:grumbel/uinpp";
    uinpp.inputs.nixpkgs.follows = "nixpkgs";
    uinpp.inputs.flake-utils.follows = "flake-utils";
    uinpp.inputs.strutcpp.follows = "strutcpp";
    uinpp.inputs.logmich.follows = "logmich";
    uinpp.inputs.tinycmmc.follows = "tinycmmc";

    unsebu.url = "github:grumbel/unsebu";
    unsebu.inputs.nixpkgs.follows = "nixpkgs";
    unsebu.inputs.flake-utils.follows = "flake-utils";
    unsebu.inputs.tinycmmc.follows = "tinycmmc";
    unsebu.inputs.logmich.follows = "logmich";

    yaini.url = "github:grumbel/yaini";
    yaini.inputs.nixpkgs.follows = "nixpkgs";
    yaini.inputs.flake-utils.follows = "flake-utils";
    yaini.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils, argpp, tinycmmc, strutcpp, logmich, uinpp, unsebu, yaini }:
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
              fmt
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
              libxkbcommon
              pcre
              (pkgs.python3.withPackages (p: [
                p.dbus-python
              ]))
              util-linux
              xorg.libX11
              xorg.libXdmcp
              xorg.libXtst
            ] ++ [
              argpp.packages.${system}.default
              logmich.packages.${system}.default
              strutcpp.packages.${system}.default
              uinpp.packages.${system}.default
              unsebu.packages.${system}.default
              yaini.packages.${system}.default
              tinycmmc.packages.${system}.default
            ];
          };
        };
      }
    );
}
