copy_file_to_rootfs() {
    install -d ${D}/boot

    if ls ${DEPLOY_DIR_IMAGE}/uImage-${MACHINE}.dtb 1> /dev/null 2>&1; then
        cp -Lr ${DEPLOY_DIR_IMAGE}/uImage-${MACHINE}.dtb ${IMAGE_ROOTFS}/boot
    fi
    
    if ls ${DEPLOY_DIR_IMAGE}/uImage-${MACHINE}.bin 1> /dev/null 2>&1; then
        cp -Lr ${DEPLOY_DIR_IMAGE}/uImage-${MACHINE}.bin ${IMAGE_ROOTFS}/boot
    fi
}
ROOTFS_POSTPROCESS_COMMAND += "copy_file_to_rootfs;"

