#!/bin/bash

base_dir=$(pwd)

# Function to run 'make clean'
clean_make(){
    echo "Cleaning using Makefile in $1"
    cd "$1"
    make clean
    if [ $? -ne 0 ]; then
        echo "Make clean failed in $1"
    fi
    cd "$base_dir"
}

# Function to handle both CMake and Makefile builds
clean_cmake(){
    echo "Cleaning using CMake in $1"
    cd "$1"
    if [ -f "CMakeLists.txt" ]; then
        cmake --build . --target clean
        if [ $? -ne 0 ]; then
            echo "CMake clean failed in $1"
        fi
    fi
    cd "$base_dir"
}

# Search and clean directories based on Makefile or CMakeLists.txt
search_and_clean(){
    local dir=$1

    if [ -f "$dir/CMakeLists.txt" ]; then
        clean_cmake "$dir"
    elif [ -f "$dir/Makefile" ]; then
        clean_make "$dir"
    fi
}

# Recursively iterate over directories to find and clean Makefile/CMake projects
recursive_clean(){
    local current_dir=$1
    for subdir in "$current_dir"/*; do
        if [ -d "$subdir" ]; then
            search_and_clean "$subdir"
            recursive_clean "$subdir"
        fi
    done
}

# Start cleaning process from the base directory
recursive_clean "$base_dir"

echo "All MIRA implementations cleaned."
