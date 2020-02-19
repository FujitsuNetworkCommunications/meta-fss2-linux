SUMMARY = "A basic image init script for setup SSD"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
SRC_URI = "file://${MACHINE}/init_setup_ssd.sh \ 
           file://${MACHINE}/make_fdisk_mkfs.sh \
           file://${MACHINE}/make_rootfs.sh \
           file://${MACHINE}/mount_usb.sh \
           file://${MACHINE}/sda.out \
           "

PR = "r3"

do_install() {
        install -m 0755 ${WORKDIR}/${MACHINE}/init_setup_ssd.sh ${D}/init_setup_ssd.sh
        install -m 0755 ${WORKDIR}/${MACHINE}/make_fdisk_mkfs.sh ${D}/make_fdisk_mkfs.sh
        install -m 0755 ${WORKDIR}/${MACHINE}/make_rootfs.sh ${D}/make_rootfs.sh
        install -m 0755 ${WORKDIR}/${MACHINE}/mount_usb.sh ${D}/mount_usb.sh
        install -m 0755 ${WORKDIR}/${MACHINE}/sda.out ${D}/sda.out
}

inherit allarch

FILES_${PN} += " /init_setup_ssd.sh /make_fdisk_mkfs.sh /make_rootfs.sh /mount_usb.sh /sda.out"
