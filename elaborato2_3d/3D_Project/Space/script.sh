#!/bin/bash

# List of required libraries
LIBS=(libglu1-mesa-dev libglew-dev libgl1-mesa-dev freeglut3-dev libassimp-dev)

# Verify presence of required libraries
for lib in ${LIBS[@]}; do
    if ! dpkg -s $lib > /dev/null 2>&1; then
        echo "Installing $lib..."
        sudo apt-get install -y $lib
    fi
done

# Compile the code using the provided makefile
make

# Run the resulting executable
./exe

