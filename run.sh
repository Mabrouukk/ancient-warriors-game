#!/bin/bash
# Ancient Warriors Game - Easy Run Script

cd "$(dirname "$0")"

# Build if not already built
if [ ! -f "src/P1600_1977" ]; then
    echo "Building game..."
    g++ -std=c++17 src/P1600_1977.cpp -o src/P1600_1977 -framework OpenGL -framework GLUT -Wno-deprecated
fi

# Run the game
echo "Starting Ancient Warriors Game..."
./src/P1600_1977
