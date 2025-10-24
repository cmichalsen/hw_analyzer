#!/usr/bin/env bash
set -e

echo "🔨 Building HW Analyzer Backend..."

# Create build directory
mkdir -p backend/build
cd backend/build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

echo "✅ Backend built successfully!"
echo "📍 Binary location: backend/build/hw_analyzer_backend"
