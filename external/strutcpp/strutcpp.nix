{ stdenv
, lib
, cmake
, gtest
, tinycmmc
}:

stdenv.mkDerivation {
  pname = "strutcpp";
  version = "0.0.0";

  src = lib.cleanSource ./.;

  cmakeFlags = [
    "-DBUILD_TESTS=ON"
  ];

  doCheck = true;

  checkPhase = ''
    make test ARGS="-v"
  '';

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    tinycmmc
    gtest
  ];
}
