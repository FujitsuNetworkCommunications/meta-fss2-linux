#
# Yocto recipe to build a kernel module out of the kernel tree
# kernmodule.bb  
# Marco Cavallini - KOAN sas - www.koansoftware.com
#

DESCRIPTION = "Kernel driver to allow cma allocation"
SECTION = "examples"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=ba6dd14d0438673e9371550a9786143f"
PR = "r3"

inherit module systemd

####################################################################################
# The driver source file(s) should be located in a separate component. It is 
# located in the recipe's file directory here for readability only.
####################################################################################
SRC_URI =   	"file://cma-alloc.c \
                 file://cma-alloc.h \
                 file://Makefile \
                 file://COPYING \
                 file://cma-alloc-driver.service \
                 file://cma-alloc-driver-installer.sh \
		"
S = "${WORKDIR}"
FILES_${PN} += "${LOCAL_FNC_DIR}/cma-alloc-driver/ \
		${install_headers_dir}/*"

SYSTEMD_SERVICE_${PN} = "cma-alloc-driver.service"
install_headers_dir = "/usr/include/${PN}"

do_install_append () {
    install -m 0755 -d ${D}${install_headers_dir}
    cp -Rfp ${WORKDIR}/cma-alloc.h ${D}${install_headers_dir}/

    install -d ${D}${LOCAL_FNC_DIR}/cma-alloc-driver/
    install -m 0755 ${WORKDIR}/cma-alloc-driver-installer.sh ${D}${LOCAL_FNC_DIR}/cma-alloc-driver/

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/cma-alloc-driver.service ${D}${systemd_unitdir}/system
}

SRCREV = "f35892b3641ff3919af270825751c67f102c0d1c"
