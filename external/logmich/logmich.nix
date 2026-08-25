{ stdenv
, lib
, cmake
, fmt_8
, tinycmmc
}:

stdenv.mkDerivation {
  pname = "logmich";
  version = "0.2.0";

  src = lib.cleanSource ./.;

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    tinycmmc
  ];

  propagatedBuildInputs = [
    fmt_8
  ];
}
