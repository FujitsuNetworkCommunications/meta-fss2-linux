DESCRIPTION = "Google's framework for writing C++ tests"
HOMEPAGE = "https://github.com/google/googletest"
SECTION = "libs"
LICENSE = "BSD-3-Clause"
# Once we upgrade to gtest 1.8.0, gtest and gmock have merged.
#LIC_FILES_CHKSUM = "file://googlemock/LICENSE;md5=cbbd27594afd089daa160d3a16dd515a \
#                    file://googletest/LICENSE;md5=cbbd27594afd089daa160d3a16dd515a"
LIC_FILES_CHKSUM = "file://LICENSE;md5=cbbd27594afd089daa160d3a16dd515a"

#PROVIDES += "gmock"
# FNC Modification that I've long forgotten the reason it was needed.
CXXFLAGS += " -gdwarf-2 -O0"

SRC_URI = "\
    http://googletest.googlecode.com/files/${BPN}-${PV}.zip \
"

SRC_URI[md5sum] = "2d6ec8ccdf5c46b05ba54a9fd1d130d7"
SRC_URI[sha256sum] = "247ca18dd83f53deb1328be17e4b1be31514cedfc1e3424f672bf11fd7e0d60d"

PR = "r1"

inherit cmake

# FNC Addition: install instructions are missing in cmake for 1.7.0 but
# are back in 1.8.0.
do_install (){
    mkdir -p ${D}${includedir}
    mkdir -p ${D}${libdir}
    cp -r ${S}/include/* ${D}${includedir}/
    cp -r ${B}/libgtest.a ${D}${libdir}/
    cp -r ${B}/libgtest_main.a ${D}${libdir}/
}


ALLOW_EMPTY_${PN} = "1"
ALLOW_EMPTY_${PN}-dbg = "1"

RDEPENDS_${PN}-dev += "${PN}-staticdev"

BBCLASSEXTEND = "native nativesdk"
