SUMMARY="helloworld-grpc example client and server"
LICENSE = "Apache-2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

DEPENDS = "grpc"

SRC_URI = "git://github.com/FujitsuNetworkCommunications/helloworld-grpc.git;protocol=https"
SRCREV = "4e0f3b22d9b3f6be1a45717184f84aba3dff9fa7"
PV = "1.0"
PR = "r0"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

BBCLASSEXTEND += "native"
