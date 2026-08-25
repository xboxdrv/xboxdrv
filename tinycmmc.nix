{ self
, stdenv
, lib
, cmake
, tinycmmc_lib
}:

stdenv.mkDerivation {
  pname = "tinycmmc";
  version = tinycmmc_lib.versionFromFile self;

  src = lib.cleanSource ./.;

  nativeBuildInputs = [
    cmake
  ];
}
