{ stdenv
, lib
, cmake
, gtest
, tinycmmc
}:

stdenv.mkDerivation {
  pname = "argpp";
  version = "1.0.0";

  src = lib.cleanSource ./.;

  cmakeFlags = [
    "-DBUILD_TESTS=ON"
  ];

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    gtest
    tinycmmc
  ];
}
