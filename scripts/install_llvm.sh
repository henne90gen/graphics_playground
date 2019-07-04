#!/usr/bin/env bash
set -eou pipefail

MIRROR="http://releases.llvm.org"

# Determine architecture
ARCH=$(dpkg --print-architecture)

# Default platform and download type
PLATFORM="linux-gnu"
PACKAGE_BASE="clang+llvm"
DOWNLOAD_TYPE="tar.xz"

# Match architecture with LLVM naming
case "${ARCH}" in
"amd64") ARCH="x86_64";;
esac

# Match quirks
case "${ARCH}:${LLVM_VERSION}" in
"x86_64:2.9")
  PLATFORM="linux.tar"
  DOWNLOAD_PLATFORM="linux"
  DOWNLOAD_TYPE="tar.bz2"
  ;;
"x86_64:3.0")
  PLATFORM="linux-debian"
  DOWNLOAD_TYPE="tar.gz"
  ;;
"x86_64:3.1")
  PLATFORM="linux-ubuntu_12.04"
  DOWNLOAD_TYPE="tar.gz"
  ;;
"x86_64:3.2")
  PLATFORM="linux-ubuntu-12.04"
  DOWNLOAD_TYPE="tar.gz"
  ;;
"x86_64:3.3")
  ARCH="amd64"
  PLATFORM="debian6"
  DOWNLOAD_TYPE="tar.bz2"
  ;;
"x86_64:3.4.2")
  PLATFORM="linux-gnu-ubuntu-14.04"
  DOWNLOAD_TYPE="xz"
  ;;
"x86_64:3.5.2")
  DOWNLOAD_PLATFORM="linux-gnu-ubuntu-14.04"
  ;;
"x86_64:3.6.2")
  PLATFORM="linux-gnu-ubuntu-15.04"
  ;;
"x86_64:3.7.1")
  PLATFORM="linux-gnu-ubuntu-15.10"
  ;;
"x86_64:3.8.1")
  PLATFORM="linux-gnu-debian8"
  ;;
"x86_64:3.9.1")
  PLATFORM="linux-gnu-debian8"
  ;;
"x86_64:4.0."*)
  PLATFORM="linux-gnu-debian8"
  ;;
"x86_64:5.0."*)
  PLATFORM="linux-gnu-ubuntu-16.04"
  ;;
"x86_64:6.0."*)
  PLATFORM="linux-gnu-ubuntu-16.04"
  ;;
"x86_64:7.0."*)
  PLATFORM="linux-gnu-ubuntu-18.04"
  ;;
"x86_64:8.0."*)
  PLATFORM="linux-gnu-ubuntu-18.04"
  ;;
esac

if [ -z ${DOWNLOAD_PLATFORM+x} ]; then
  DOWNLOAD_PLATFORM="${PLATFORM}"
fi

# Set install target and download file
TARGET="${PACKAGE_BASE}-${LLVM_VERSION}-${ARCH}-${PLATFORM}"
DOWNLOAD="${PACKAGE_BASE}-${LLVM_VERSION}-${ARCH}-${DOWNLOAD_PLATFORM}.${DOWNLOAD_TYPE}"
DOWNLOAD_FILE="llvm.${DOWNLOAD_TYPE}"

# Download
echo "Downloading ${DOWNLOAD}"
wget -nv -O "${DOWNLOAD_FILE}"     "${MIRROR}/${LLVM_VERSION}/${DOWNLOAD}"
wget -nv -O "${DOWNLOAD_FILE}.sig" "${MIRROR}/${LLVM_VERSION}/${DOWNLOAD}.sig"

# Verify
gpg --batch --verify "${DOWNLOAD_FILE}.sig" "${DOWNLOAD_FILE}"

# Install
echo "Installing ${TARGET}"
tar xf "${DOWNLOAD_FILE}"
cp -a "${TARGET}/"* "/usr/local/"

# Cleanup
rm -rf "${DOWNLOAD_FILE}" "${TARGET}/" install.sh