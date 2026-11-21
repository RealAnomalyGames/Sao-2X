#!/usr/bin/env bash

set -e

echo "===================================="
echo "Sao-2X Build System"
echo "===================================="

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release

ctest --test-dir build -C Release --output-on-failure

echo
echo "Sao-2X build completed successfully."
