{
  pkgs,
  inputs,
  ...
}:
inputs.treefmt-nix.lib.mkWrapper pkgs {
  projectRootFile = "flake.nix";
  programs = {
    # nix
    nixfmt.enable = true;
    deadnix.enable = true;
    statix.enable = true;
    # Shell
    shellcheck.enable = true;
    shfmt.enable = true;
    # Cmake
    cmake-format = {
      enable = true;
      includes = [
        # Get cmake lists at all levels
        "CMakeLists.txt"
        "**/CMakeLists.txt"
      ];
    };
    # C, C++
    clang-format.enable = true;
    # Various
    prettier = {
      enable = true;
      includes = [
        ".clang-*"
        "*.yml"
        "*.md"
      ];
    };
  };
  settings.global.excludes = [
    "build"
    "*.pdf"
  ];
}
