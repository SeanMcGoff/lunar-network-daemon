# Lunar Network Daemon Structure

- accept packet
- async sleep for (relevant time-delay) seconds
- fast "random" number gen for bit flipping
- release packet

## Building

The project requires CMake, a CMake backend (Make or Ninja), and a C++ compiler (GCC or Clang).

Once these have been obtained the project can be build using

```sh
cmake -S . -B build/; cmake --build build/
```

This will build a debug build by default. To build a release build for deployment before something like a demo or what have you, run the following:

```sh
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/; cmake --build build/
```

Note that CMake will not regenerate the build cache when changing flags, so if going from a normal to release build or back one must remove the build cache directory, by default `build`.

A neat way to remove all files not tracked by git is

```sh
git clean -fdx
```

## Developing

Please install

- `clangd` for messages in editor. It should also give in editor formatting
- `clang-tidy` for linting files during build
- `clang-format` for formatting the files.

Adhering to the formatting and linting requirements will be required by CI, and code cannot be merged unless it meets the requirements.

## Nix

This project has a nix flake!

It is structured using [Blueprint](https://github.com/numtide/blueprint).

It has a development shell with required deps, all formatters configured with [Treefmt](https://github.com/numtide/treefmt-nix), and packages for building both in Release and Debug modes.

To build a debug build use

```sh
nix build .#lunar-network-daemon-debug
```

and release is

```sh
nix build .#lunar-network-daemon
```
