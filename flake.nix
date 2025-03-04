{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    blueprint.url = "github:numtide/blueprint";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    inputs:
    inputs.blueprint {
      inherit inputs;
      prefix = "./nix/";
      nixpkgs.overlays = [
        # Pin nlohmann_json to 3.11.3
        (_final: prev: {
          nlohmann_json = prev.nlohmann_json.overrideAttrs (_finalAttrs: {
            version = "3.11.3";
          });
        })
      ];
    };
}
