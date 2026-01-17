#!/bin/bash
set -e

# Update package list
sudo apt-get -qq -y  update

# Install necessary dependencies
sudo apt-get -qq -y install build-essential libssl-dev libbz2-dev \
                            libreadline-dev libsqlite3-dev zlib1g-dev \
							libffi-dev wget tar

# Download Python 2.7.x source code (adjust version number as needed)
PYTHON_VERSION=2.7.18
rm -rf "Python-$PYTHON_VERSION" "Python-$PYTHON_VERSION.tgz"
wget -q "https://www.python.org/ftp/python/$PYTHON_VERSION/Python-$PYTHON_VERSION.tgz" \
    -O "Python-${PYTHON_VERSION}.tgz"

# Extract the source code
tar -xzf Python-$PYTHON_VERSION.tgz
cd Python-$PYTHON_VERSION

export CFLAGS="${CFLAGS} -w"

# Configure the build
./configure  \
    --prefix=/usr/local \
    --disable-shared \
    --disable-ipv6 \
    --without-doc-strings \
    --without-ensurepip \
    --without-computed-gotos \
    --without-pymalloc \
    --disable-optimizations \
    --quiet

# Compile and install
make -j "$(nproc)" -s
sudo make altinstall
sudo ln -fns /usr/local/bin/python2.7 /usr/local/bin/python
sudo ln -fns /usr/local/bin/python2.7 /usr/local/bin/python2
# Clean up
cd ..
rm -rf Python-$PYTHON_VERSION Python-$PYTHON_VERSION.tgz

# Verify the installation
python2.7 --version
