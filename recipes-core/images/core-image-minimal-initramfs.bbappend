PACKAGE_INSTALL = "u-boot-qoriq busybox udev base-files base-passwd initscripts initramfs-boot keymaps util-linux ${ROOTFS_BOOTSTRAP_INSTALL}"

inherit image_types_uboot
IMAGE_FSTYPES += "tar.gz ext2.gz ext2.gz.u-boot"

