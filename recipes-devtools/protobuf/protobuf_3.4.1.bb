SUMMARY = "Protocol Buffers - structured data serialisation mechanism"
DESCRIPTION = "Protocol Buffers are a way of encoding structured data in an \
efficient yet extensible format. Google uses Protocol Buffers for almost \
all of its internal RPC protocols and file formats."
HOMEPAGE = "http://code.google.com/p/protobuf/"
SECTION = "console/tools"
LICENSE = "BSD-3-Clause"

DEPENDS = "zlib"
DEPENDS_append_class-target  = " protobuf-native"

LIC_FILES_CHKSUM = "file://LICENSE;md5=35953c752efc9299b184f91bef540095"

SRC_URI = "git://github.com/google/protobuf.git"
SRCREV = "b04e5cba356212e4e8c66c61bbe0c3a20537c5b9"
PR = "r1"

S = "${WORKDIR}/git"

EXTRA_OECONF += " --with-protoc=echo"

inherit autotools

do_install_append() {
    install -m 0755 ${S}/src/.libs/libprotoc.so.14.0.0 ${D}${libdir}
    rm ${D}${libdir}/*.a ${D}${libdir}/*.la
}

BBCLASSEXTEND = "native nativesdk"
