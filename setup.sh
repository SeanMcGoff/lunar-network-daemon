#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

echo "🚀 Setting up Lunar Network Daemon..."

# Step 1: Install Dependencies
echo "🔧 Installing Docker and Docker Compose..."
sudo apt update
sudo apt install -y docker.io docker-compose

# Enable and start Docker service
sudo systemctl enable --now docker

# Verify installations
echo "✅ Docker Version: $(docker --version)"
echo "✅ Docker Compose Version: $(docker-compose --version)"

# Step 2: Set Up Firewall Rules (if UFW is active)
if sudo ufw status | grep -q "Status: active"; then
    echo "🔒 Configuring firewall..."
    sudo ufw allow 80/tcp
    sudo ufw allow 8080/tcp
    sudo ufw reload
    echo "✅ Firewall rules updated."
else
    echo "⚠️ UFW is not active, skipping firewall setup."
fi

# Step 3: Build and Start Containers
echo "🐳 Building and starting Docker containers..."
docker-compose up --build -d

# Step 4: Verify Services
echo "🔍 Checking running containers..."
docker ps

# Step 5: Test Accessibility
echo "🌐 Testing Nginx availability..."
sleep 5  # Give the services some time to start

if curl -I http://localhost 2>&1 | grep -q "HTTP/1.1 200"; then
    echo "✅ Nginx is up and running!"
else
    echo "❌ Nginx is not responding. Check logs with: docker logs nginx"
fi

echo "✅ Lunar Network Daemon setup completed! 🎉"
echo "➡️ Access it via: ..."
