DESCRIPTION = "Google's framework for mocking C++ classes"
HOMEPAGE = "https://github.com/google/googlemock"
SECTION = "libs"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=cbbd27594afd089daa160d3a16dd515a"

# FNC comment: once we upgrade Yocto/OpenEmbedded that includes
# gtest-1.8.0, gmock library will be provided by gtest and this recipe
# should be deleted.

# FNC Modification that I've long forgotten the reason it was needed.
CXXFLAGS += " -gdwarf-2 -O0"

SRC_URI = "\
    http://googlemock.googlecode.com/files/${BPN}-${PV}.zip \
"

SRC_URI[md5sum] = "073b984d8798ea1594f5e44d85b20d66"
SRC_URI[sha256sum] = "26fcbb5925b74ad5fc8c26b0495dfc96353f4d553492eb97e85a8a6d2f43095b"

inherit cmake

# FNC Addition: install instructions are missing in cmake for 1.7.0 but
# are back in 1.8.0.
do_install (){
    mkdir -p ${D}${includedir}
    mkdir -p ${D}${libdir}
    cp -r ${S}/include/* ${D}${includedir}/
    cp -r ${B}/libgmock.a ${D}${libdir}/
}

ALLOW_EMPTY_${PN} = "1"
ALLOW_EMPTY_${PN}-dbg = "1"

RDEPENDS_${PN}-dev += "${PN}-staticdev"

BBCLASSEXTEND = "native nativesdk"
