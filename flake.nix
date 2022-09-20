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

    unsebu.url = "github:Grumbel/unsebu";
    unsebu.inputs.nixpkgs.follows = "nixpkgs";
    unsebu.inputs.flake-utils.follows = "flake-utils";
    unsebu.inputs.tinycmmc.follows = "tinycmmc";
    unsebu.inputs.logmich.follows = "logmich";

    yaini.url = "github:Grumbel/yaini";
    yaini.inputs.nixpkgs.follows = "nixpkgs";
    yaini.inputs.flake-utils.follows = "flake-utils";
    yaini.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils, argpp, tinycmmc, strutcpp, logmich, uinpp, unsebu, yaini }:
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

      in {
        packages = rec {
          default = xboxdrv;

          xboxdrv = pkgs.stdenv.mkDerivation {
            pname = "xboxdrv";
            version = project_version_from_file;
            src = nixpkgs.lib.cleanSource ./.;
            postPatch = ''
              echo "v${project_version_from_file}" > VERSION
            '';
            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config
            ];
            buildInputs = with pkgs; [
              at-spi2-core
              bluez
              dbus-glib
              epoxy
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
              python3
              python3Packages.dbus-python
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
