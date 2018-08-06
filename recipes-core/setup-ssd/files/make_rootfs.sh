#!/bin/sh

echo "Start to copy files to bank 1"
mkdir /mnt/ssd1
mount /dev/sda1 /mnt/ssd1

cd /mnt/ssd1
mkdir boot
mkdir home
mount /dev/sda5 boot
mount /dev/sda9 home

mkdir boot/ucnt
cp /mnt/usb/T600/fp01.bin boot/ucnt
#shall be remove
cp /mnt/usb/T600/fp01.bin .

cp /mnt/usb/T600/core-image-minimal-t600.rootfs.tar.gz .
tar zxf core-image-minimal-t600.rootfs.tar.gz
sync
sleep 1
echo "Remove file core-image-minimal"
rm core-image-minimal-t600.rootfs.tar.gz
sync

echo "Umount sda5, sda9 and sda1 "
cd ..
umount /dev/sda5
umount /dev/sda9
sleep 1
umount /dev/sda1

e2fsck /dev/sda1
e2fsck /dev/sda5
e2fsck /dev/sda9

echo "Mount sda5, sda9 and sda1 again to create the journaling"
mount /dev/sda1 /mnt/ssd1
mount /dev/sda5 /mnt/ssd1/boot
mount /dev/sda9 /mnt/ssd1/home

echo "Umount sda5, sda9 and sda1 again"
umount /dev/sda5
umount /dev/sda9
sleep 1
umount /dev/sda1

echo "Start to copy files to bank 2"
mkdir /mnt/ssd2
mount /dev/sda2 /mnt/ssd2

cd /mnt/ssd2
mkdir boot
mkdir home
mount /dev/sda6 boot
mount /dev/sda9 home

mkdir boot/ucnt
cp /mnt/usb/T600/fp01.bin boot/ucnt/
#shall be remove
cp /mnt/usb/T600/fp01.bin .

cp /mnt/usb/T600/core-image-minimal-t600.rootfs.tar.gz .
tar zxf core-image-minimal-t600.rootfs.tar.gz
sync
sleep 1
echo "Remove file core-image-minimal"
rm core-image-minimal-t600.rootfs.tar.gz
sync

echo "Umount sda6, sda9 and sda2 "
cd ..
umount /dev/sda6
umount /dev/sda9
sleep 1
umount /dev/sda2

e2fsck /dev/sda2
e2fsck /dev/sda6
e2fsck /dev/sda9

echo "Mount sda6, sda9 and sda2 again to create the journaling"
mount /dev/sda2 /mnt/ssd2
mount /dev/sda6 /mnt/ssd2/boot
mount /dev/sda9 /mnt/ssd2/home

echo "Umount sda6, sda9 and sda2 again"
umount /dev/sda6
umount /dev/sda9
sleep 1
umount /dev/sda2

echo "Done"

