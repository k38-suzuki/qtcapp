#!/bin/sh

# MinIO
cd ~
wget https://dl.min.io/server/minio/release/linux-amd64/archive/minio_20230322063624.0.0_amd64.deb -O minio.deb
sudo dpkg -i minio.deb

# AWS SDK for C++
sudo --preserve-env=DEBIAN_FRONTEND,TZ \
apt-get -y install \
libcurl4-openssl-dev \
libssl-dev \
uuid-dev \
zlib1g-dev \
libpulse-dev \

cd ~
git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp

# AWS S3
mkdir sdk_build
cd sdk_build
cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/usr/local/ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_ONLY="s3"
make -j8
sudo make install
cd ~
rm -rf sdk_build

# AWS STS
# mkdir sdk_build
# cd sdk_build
# cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/usr/local/ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_ONLY="sts"
# make -j8
# sudo make install
# cd ~
# rm -rf sdk_build

rm -rf aws-sdk-cpp
