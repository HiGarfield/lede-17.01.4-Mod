#!/bin/bash
set -e

# Update package list
sudo apt -qqq update

# Install necessary dependencies
sudo apt -qqq install build-essential libssl-dev libbz2-dev \
    libreadline-dev libsqlite3-dev wget curl llvm \
    libxml2-dev libxslt1-dev zlib1g-dev libffi-dev

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
./configure

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
