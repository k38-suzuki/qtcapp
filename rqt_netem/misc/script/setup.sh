#!/bin/sh

LOGNAME=$(logname)
sudo echo "$LOGNAME    ALL=NOPASSWD: /sbin/modprobe" >> /etc/sudoers
sudo echo "$LOGNAME    ALL=NOPASSWD: /sbin/ip" >> /etc/sudoers
sudo echo "$LOGNAME    ALL=NOPASSWD: /sbin/rmmod" >> /etc/sudoers
sudo echo "$LOGNAME    ALL=NOPASSWD: /sbin/tc" >> /etc/sudoers
