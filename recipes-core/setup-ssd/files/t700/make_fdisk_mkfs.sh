#!/bin/sh

#try to partition the SSD
# /                5G
# /standby         5G
# /boot          200M
# /standby/boot  200M
# /var/log       300M
# /var/shared     20G
# home           200M

echo "Use sfdisk to partition sda..."
sfdisk /dev/sda < sda.out

echo "Finish partition and start to format..."
mkfs.ext4 -F /dev/sda1
mkfs.ext4 -F /dev/sda2
mkfs.ext2 -F /dev/sda5
mkfs.ext2 -F /dev/sda6
mkfs.ext4 -F /dev/sda8
mkfs.ext4 -F /dev/sda9

echo "Format partition done."
