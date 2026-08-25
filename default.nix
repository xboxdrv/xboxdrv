# Copyright (C) 2022 Ingo Ruhnke <grumbel@gmail.com>
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

{ nixpkgs
, flake-utils
}:
let
  versionFromFileOr = project: fallback-version:
    let
      version_content = nixpkgs.lib.fileContents "${project}/VERSION";
      project_version =
        if (((builtins.substring 0 1) version_content) != "v") then
          "${fallback-version}-${nixpkgs.lib.substring 0 8 project.lastModifiedDate}-${project.shortRev or "dirty"}"
        else
          builtins.substring 1 ((builtins.stringLength version_content) - 1) version_content;
    in
      project_version;

  versionFromFile = project: versionFromFileOr project "unknown";

  eachSystem  = (func:
    flake-utils.lib.eachSystem (flake-utils.lib.defaultSystems ++ [ "x86_64-windows" "i686-windows" ]) func
  );

  eachWin32System  = (func:
    flake-utils.lib.eachSystem [ "x86_64-windows" "i686-windows" ] func
  );

  pkgsFromSystem = (system:
    if system == "x86_64-windows" then nixpkgs.legacyPackages.x86_64-linux.pkgsCross.mingwW64
    else if system == "i686-windows" then nixpkgs.legacyPackages.x86_64-linux.pkgsCross.mingw32
    else nixpkgs.legacyPackages.${system}
  );

  eachSystemWithPkgs = (func:
    eachSystem (system: (func (pkgsFromSystem system)))
  );

  eachWin32SystemWithPkgs  = (func:
    eachWin32System (system: (func (pkgsFromSystem system)))
  );

  lib = {
    inherit
      versionFromFileOr
      versionFromFile
      eachSystem
      eachWin32System
      pkgsFromSystem
      eachSystemWithPkgs
      eachWin32SystemWithPkgs
    ;
  };
in
lib
