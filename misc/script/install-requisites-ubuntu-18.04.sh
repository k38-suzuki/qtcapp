#!/bin/sh

sudo apt-get -y install \
build-essential \
cmake-curses-gui \
libboost-all-dev \
qt5-default \
libqt5x11extras5-dev \
libqt5svg5-dev \
qt5-style-plugins

LOGNAME=$(logname)

echo "$LOGNAME    ALL=NOPASSWD: /sbin/modprobe" >> /etc/sudoers
echo "$LOGNAME    ALL=NOPASSWD: /sbin/ip" >> /etc/sudoers
echo "$LOGNAME    ALL=NOPASSWD: /sbin/rmmod" >> /etc/sudoers
echo "$LOGNAME    ALL=NOPASSWD: /sbin/tc" >> /etc/sudoers
