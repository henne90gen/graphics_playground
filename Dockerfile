FROM debian:stable

# Install dependencies
RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
        autoconf automake cmake dpkg-dev file git make patch \
        libc-dev libc++-dev libgcc-6-dev libstdc++-6-dev  \
        dirmngr gnupg2 lbzip2 wget xz-utils; \
    rm -rf /var/lib/apt/lists/*

# Signing keys
ENV GPG_KEYS 09C4E7007CB2EFFB A2C794A986419D8A B4468DF4E95C63DC D23DD2C20DD88BA2 8F0871F202119294 0FC3042E345AD05D

# Retrieve keys
RUN gpg --batch --keyserver ha.pool.sks-keyservers.net --recv-keys $GPG_KEYS

# Version info
ENV LLVM_RELEASE 8
ENV LLVM_VERSION 8.0.0
ENV CMAKE_VERSION 3.14
ENV CMAKE_BUILD 5

# Install Clang and LLVM
COPY scripts/install_llvm.sh .
RUN ./install_llvm.sh

# Install CMake
COPY scripts/install_cmake.sh .
RUN ./install_cmake.sh

ADD . /app
WORKDIR /app
