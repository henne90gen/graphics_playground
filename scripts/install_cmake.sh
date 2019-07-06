#!/usr/bin/env bash

apt-get purge -y cmake

mkdir ~/temp
cd ~/temp
wget -nv --no-check-certificate https://cmake.org/files/v$CMAKE_VERSION/cmake-$CMAKE_VERSION.$CMAKE_BUILD.tar.gz
tar -xzvf cmake-$CMAKE_VERSION.$CMAKE_BUILD.tar.gz
cd cmake-$CMAKE_VERSION.$CMAKE_BUILD/

./bootstrap
make -j4
make install

cd ~/
rm -rf ~/temp

cmake --version
