#!/bin/sh

#try to partition the SSD
# /                5G
# /standby         5G
# /boot          200M
# /standby/boot  200M
# /var/log       300M
# /var/shared     20G
# home           200M

sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/sda
  o # clear the in memory partition table
  n # new partition
  p # primary partition
  1 # partition number 1
   # default - start at beginning of disk
  +5G # 5 GB parttion
  n
  p
  2
   # default
  +5G # 5 GB parttion
  n
  e
  3
   # default - start at beginning of disk
   # default, extend partition to end of disk
  n
  l
   # default 
  +200M # 200 MB parttion
  n
  l
   # default
  +200M # 200 MB parttion
  n
  l
   # default
  +300M # 300 MB parttion
  n
  l
   # default
  +18G # 18 GB parttion
  n
  l
   # default
  +200M # 200 MB parttion
  p # print the in-memory partition table
  w # write the partition table
  q
EOF

echo "Finish partition and start to format..."
mkfs -t ext4 /dev/sda1
mkfs -t ext4 /dev/sda2
mkfs -t ext2 /dev/sda5
mkfs -t ext2 /dev/sda6
mkfs -t ext4 /dev/sda8
mkfs -t ext4 /dev/sda9

echo "Format partition done."
