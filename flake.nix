{
  description = "Xbox360 USB Gamepad Userspace Driver";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
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
    strutcpp.inputs.flake-utils.follows = "flake-utils";
    strutcpp.inputs.tinycmmc.follows = "tinycmmc";

    logmich.url = "github:logmich/logmich";
    logmich.inputs.nixpkgs.follows = "nixpkgs";
    logmich.inputs.flake-utils.follows = "flake-utils";
    logmich.inputs.tinycmmc.follows = "tinycmmc";

    uinpp.url = "github:Grumbel/uinpp";
    uinpp.inputs.nixpkgs.follows = "nixpkgs";
    uinpp.inputs.flake-utils.follows = "flake-utils";
    uinpp.inputs.strutcpp.follows = "strutcpp";
    uinpp.inputs.logmich.follows = "logmich";
    uinpp.inputs.tinycmmc.follows = "tinycmmc";

    yaini.url = "github:Grumbel/yaini";
    yaini.inputs.nixpkgs.follows = "nixpkgs";
    yaini.inputs.flake-utils.follows = "flake-utils";
    yaini.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils, argpp, tinycmmc, strutcpp, logmich, uinpp, yaini }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        project_version_from_file = let
          version_file = pkgs.lib.fileContents ./VERSION;
          project_has_version = ((builtins.substring 0 1) version_file) == "v";
          project_version = if !project_has_version
                            then ("${nixpkgs.lib.substring 0 8 self.lastModifiedDate}-${self.shortRev or "dirty"}")
                            else (builtins.substring 1 ((builtins.stringLength version_file) - 2) version_file);
        in
          project_version;

      in rec {
        packages = flake-utils.lib.flattenTree {
          xboxdrv = pkgs.stdenv.mkDerivation {
            pname = "xboxdrv";
            version = project_version_from_file;
            src = nixpkgs.lib.cleanSource ./.;
            postPatch = ''
              echo "v${project_version_from_file}" > VERSION
            '';
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.pkg-config
            ];
            buildInputs = [
              argpp.defaultPackage.${system}
              logmich.defaultPackage.${system}
              strutcpp.defaultPackage.${system}
              uinpp.defaultPackage.${system}
              yaini.defaultPackage.${system}
              tinycmmc.defaultPackage.${system}

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
              pkgs.udev
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
