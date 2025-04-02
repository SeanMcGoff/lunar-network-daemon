# ----------------------
# Stage 1: Builder
# ----------------------
# Use the NixOS base image, which provides a purely functional package manager
FROM nixos/nix as builder

# Set the working directory inside the container
WORKDIR /daemon

# Copy the Nix flake files, which define dependencies and configurations
COPY flake.nix flake.lock nix/ ./

# Update the flake to ensure the latest dependencies are pulled
RUN nix flake update

# Set up the development environment using Nix (ensures all dependencies are installed)
RUN nix develop --command true

# Copy the entire project source code into the container
COPY . .

# Build the C++ networking daemon using the Nix flake definition
# `--impure` allows access to the host environment if needed
# The `-o result` flag creates an output symlink named `result`
RUN nix build .#lunar-network-daemon --impure -o result

# Create a directory for the output binaries and config files
RUN mkdir -p /daemon/output && \
  cp result/bin/lunar-network-daemon /daemon/output/lunar-network-daemon && \
  cp config/config.json /daemon/output/config.json

# ----------------------
# Stage 2: Runtime
# ----------------------
# Use the same NixOS base image for runtime, ensuring compatibility
FROM nixos/nix as runtime

# Set the working directory inside the container
WORKDIR /daemon

# Copy only the built binary and config from the builder stage to reduce image size
COPY --from=builder /daemon/output/ /daemon/

# Expose the port the daemon listens on
EXPOSE 8080

# Define a health check to ensure the daemon is running and accessible
HEALTHCHECK --interval=30s --timeout=10s --retries=3 \
  CMD sh -c 'pgrep lunar-network-daemon || (echo "Process not running" && exit 1); nc -z localhost 8080 || (echo "Port not open" && exit 1)'

# Run the daemon using Nix and pass the configuration file as an argument
CMD ["nix", "run", ".#lunar-network-daemon", "--", "/daemon/config.json"]
