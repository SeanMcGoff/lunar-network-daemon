{ pkgs, ... }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    gcc
    cmake
    gnumake
    clang-tools
  ];
  shellHook = '''';
}
