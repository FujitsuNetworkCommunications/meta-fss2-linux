copy_file_to_rootfs() {
    install -d ${D}/boot

    cp -Lr ${DEPLOY_DIR_IMAGE}/uImage-${MACHINE}.dtb ${IMAGE_ROOTFS}/boot
    cp -Lr ${DEPLOY_DIR_IMAGE}/uImage-${MACHINE}.bin ${IMAGE_ROOTFS}/boot
}
ROOTFS_POSTPROCESS_COMMAND += "copy_file_to_rootfs;"

