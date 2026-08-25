{ stdenv
, lib
, cmake
, gtest
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
  ];

  doCheck = true;
}
