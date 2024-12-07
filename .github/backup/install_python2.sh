#!/bin/bash

# Update package list
sudo apt update

# Install necessary dependencies
sudo apt install -y build-essential libssl-dev libbz2-dev \
    libreadline-dev libsqlite3-dev wget curl llvm \
    libxml2-dev libxslt1-dev zlib1g-dev libffi-dev

# Download Python 2.7.x source code (adjust version number as needed)
PYTHON_VERSION=2.7.18
rm -rf "Python-$PYTHON_VERSION" "Python-$PYTHON_VERSION.tgz"
wget "https://www.python.org/ftp/python/$PYTHON_VERSION/Python-$PYTHON_VERSION.tgz" \
    -O "Python-${PYTHON_VERSION}.tgz"

# Extract the source code
tar -xzf Python-$PYTHON_VERSION.tgz
cd Python-$PYTHON_VERSION || exit 1

# Configure the build
./configure

# Compile and install
make -j "$(nproc)"
sudo make altinstall
sudo ln -s /usr/local/bin/python2.7 /usr/local/bin/python
sudo ln -s /usr/local/bin/python2.7 /usr/local/bin/python2
# Clean up
cd ..
rm -rf Python-$PYTHON_VERSION Python-$PYTHON_VERSION.tgz

# Verify the installation
python2.7 --version
