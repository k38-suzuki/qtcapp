#!/bin/bash

sudo apt-get -y install \
libcurl4-openssl-dev \
libssl-dev uuid-dev \
zlib1g-dev \
libpulse-dev

cd ~

git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp

mkdir sdk_build
cd sdk_build

cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/usr/local/ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_ONLY="iam"

make -j8
sudo make install

cd ~
rm -rf sdk_build
