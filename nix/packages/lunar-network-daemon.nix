{
  pname,
  flake, # maps to inputs.self
  pkgs, # a nixpkgs instance
  debug ? false,
  ...
}:
let
  inherit (pkgs) lib;
in
pkgs.stdenv.mkDerivation {
  name = pname;
  src = flake;
  nativeBuildInputs = with pkgs; [
    cmake
  ];
  buildInputs = with pkgs; [
    nlohmann_json
  ];
  preConfigure = lib.optionalString debug ''
    echo "Enabling debug build"
    export cmakeBuildType=Debug
  '';
}
