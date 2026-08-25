{
  description = "A Command Line Argument Parser for C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = flake-utils.lib.flattenTree rec {
          default = argpp;
          argpp = pkgs.callPackage ./argpp.nix {
            tinycmmc = tinycmmc.packages.${pkgs.system}.default;
          };
        };
      }
    );
}
