SUMMARY = "A basic image init script for setup SSD"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
SRC_URI = "file://init_setup_ssd.sh \ 
           file://make_fdisk_mkfs.sh \
           file://make_rootfs.sh \
           file://mount_usb.sh \
           "

PR = "r2"

do_install() {
        install -m 0755 ${WORKDIR}/init_setup_ssd.sh ${D}/init_setup_ssd.sh
        install -m 0755 ${WORKDIR}/make_fdisk_mkfs.sh ${D}/make_fdisk_mkfs.sh
        install -m 0755 ${WORKDIR}/make_rootfs.sh ${D}/make_rootfs.sh
        install -m 0755 ${WORKDIR}/mount_usb.sh ${D}/mount_usb.sh
}

inherit allarch

FILES_${PN} += " /init_setup_ssd.sh /make_fdisk_mkfs.sh /make_rootfs.sh /mount_usb.sh"
