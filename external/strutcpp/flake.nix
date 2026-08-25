{
  description = "A collection of string utilities for C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, tinycmmc }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = strutcpp;

          strutcpp = pkgs.callPackage ./strutcpp.nix {
            tinycmmc = tinycmmc.packages.${pkgs.system}.default;
          };
        };
      }
    );
}
