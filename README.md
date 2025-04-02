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

### Running

- **Dockerfile:**

  - Uses a multi-stage build (builder & runtime) to keep the final image small and efficient.
  - Builds the C++ networking daemon using Nix flakes.
  - Copies only the necessary binaries and configuration into the runtime stage.
  - Includes a health check to monitor the daemon process and port status.

- **nginx.conf:**

  - Configures Nginx as a reverse proxy to forward requests to the Lunar Network Daemon.
  - Uses an upstream block for potential load balancing across multiple daemon instances.

- **docker-compose.yml:**
  - Defines the `lunar-network-daemon` service, built from the `Dockerfile`, with 4 replicas for load balancing.
  - Defines the `nginx` service to proxy requests to the daemon.
  - Uses a health check to ensure the daemon is running before Nginx starts.
  - Establishes an internal `backend` network for secure communication between services.

### Next steps

Just run

```sh
./setup.sh
```

or manually run the following commands:

```sh
docker-compose up --build -d
```
