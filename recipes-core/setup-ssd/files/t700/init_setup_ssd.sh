#!/bin/sh

/etc/init.d/sysfs.sh
echo "Start to initialize the SSD"
/mount_usb.sh
sleep 1
/make_fdisk_mkfs.sh
sleep 3
/make_rootfs.sh

echo "End of initialize the SSD..."
