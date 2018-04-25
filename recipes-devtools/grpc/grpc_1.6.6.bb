DESCRIPTION = "A high performance, open source, general-purpose RPC framework. \
Provides gRPC libraries for multiple languages written on top of shared C core library \
(C++, Node.js, Python, Ruby, Objective-C, PHP, C#)"
HOMEPAGE = "https://github.com/grpc/grpc"
SECTION = "libs"
LICENSE = "Apache-2"

DEPENDS = "c-ares protobuf protobuf-native openssl"
DEPENDS_append_class-target = " gtest grpc-native "

LIC_FILES_CHKSUM = "file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

SRC_URI = "git://github.com/grpc/grpc.git;protocol=git;branch=v1.6.x \
           file://0001-Fix-grpc-makefile.patch"
SRCREV = "4ac92c69a4716a83c36718b712b75e3d6e168d92"
PR = "r1"

S = "${WORKDIR}/git"

inherit autotools

# GRPC has no do_configure as it is not a true autotools recipe
do_configure() {
}

do_compile() {
  oe_runmake 'prefix=${prefix}'
}

do_install() {
  oe_runmake 'prefix=${D}${prefix}' install
}

SHARED_MAKE_ARGS = "  HAS_SYSTEM_PROTOBUF=true HAS_VALID_PROTOC=true HAS_PROTOC=true AR='${AR} rcs' LD='${CXX}' LDXX='${CXX}' "

EXTRA_OEMAKE = " ${SHARED_MAKE_ARGS} GRPC_CROSS_COMPILE=true PROTOC_PLUGINS_DIR='${STAGING_BINDIR_NATIVE}' STRIP=echo "

EXTRA_OEMAKE_class-native = " ${SHARED_MAKE_ARGS} "

BBCLASSEXTEND = "native"
