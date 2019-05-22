#!/bin/bash

mkdir -p build
cd build
cmake -DRUN_CLANG_TIDY=ON ..
make
make test
