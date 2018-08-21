SUMMARY="CPU watchdog on power-pc"
LICENSE = "GNU General Public License"
LIC_FILES_CHKSUM = "file://COPYING;md5=ecc0551bf54ad97f6b541720f84d6569"

DEPENDS = ""

SRC_URI = "git://git@git88.accton.com.tw:~/fujitsu/act-fss2-cpu-watchdog_source.git;protocol=ssh"
SRCREV = "${AUTOREV}"
PV = "1.0"
PR = "r0"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

BBCLASSEXTEND += "native"
