#!/usr/bin/env bash
set -e

mkdir -p build
cd build || exit 0

cmake .. \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_C_COMPILER=/usr/bin/clang \
    -D CMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -D GPLAY_WARNINGS_AS_ERRORS=ON

cmake --build .

ctest
