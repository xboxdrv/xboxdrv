{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    logmich.url = "github:logmich/logmich";
    logmich.inputs.nixpkgs.follows = "nixpkgs";
    logmich.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc, logmich }:
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
          default = unsebu;

          unsebu = pkgs.stdenv.mkDerivation {
            pname = "unsebu";
            version = project_version_from_file;

            src = ./.;

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
              fmt_8
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
              tinycmmc.packages.${system}.default
              logmich.packages.${system}.default
            ];
          };
        };
      }
    );
}
