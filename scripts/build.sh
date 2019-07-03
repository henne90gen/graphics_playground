#!/bin/bash

mkdir -p build
cd build
#cmake -DRUN_CLANG_TIDY=ON ..
cmake -DRUN_CLANG_TIDY=OFF ..
time make -j 8 2> build_report.csv
make test
