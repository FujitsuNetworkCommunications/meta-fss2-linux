#!/bin/sh

found=0
is_pgm=0
echo "try to find file *.tar.gz"
file_list=$(ls /mnt/usb/T600/*.tar.gz)
for file_name in ${file_list}
do
    echo "find a file "${file_name}
    found=1
    break;
done

if [ "${found}" != "1" ]
then
    echo "try to find file *.PGM"
    file_list=$(ls /mnt/usb/T600/*.PGM)
    for file_name in ${file_list}
    do
        echo "find a file "${file_name}
        found=1
        is_pgm=1
        break;
    done

fi

if [ "${found}" != "1" ]
then
    echo "There is no any *.tar.gz and *.PGM files in the USB."
    echo "Stop procedure"
    exit 0
fi


echo "Start to copy files to bank 1"
mkdir /mnt/ssd1
mount /dev/sda1 /mnt/ssd1

cd /mnt/ssd1
mkdir boot
mkdir home
mount /dev/sda5 boot
mount /dev/sda9 home

mkdir boot/ucnt
cp /mnt/usb/T600/fp01.bin boot/ucnt
#shall be remove
cp /mnt/usb/T600/fp01.bin .

tar zxf ${file_name} -C /mnt/ssd1
sync
sleep 1

if [ "${is_pgm}" = "1" ]
then
    found=0

    file_list=$(ls /mnt/ssd1/*.tar.gz)
    for real_file_name in ${file_list}
    do
        echo "find a file "${real_file_name}
        found=1
        break;
    done

    if [ "${found}" != "1" ]
    then
        echo "There is no any *.tar.gz file in thg *.PGM."
        echo "Stop procedure"
        exit 0
    fi

    tar zxf ${real_file_name} -C .
    sync
    sleep 1
fi

cd boot
ln -f -s uImage-3.12.19-rt30-4.0_2+g6619b8b uImage-t600.bin
ln -f -s t600.dtb uImage-t600.dtb
cd ..
sync

echo "Umount sda5, sda9 and sda1 "
cd ..
umount /dev/sda5
umount /dev/sda9
sleep 1
umount /dev/sda1

e2fsck /dev/sda1
e2fsck /dev/sda5
e2fsck /dev/sda9

echo "Mount sda5, sda9 and sda1 again to create the journaling"
mount /dev/sda1 /mnt/ssd1
mount /dev/sda5 /mnt/ssd1/boot
mount /dev/sda9 /mnt/ssd1/home

echo "Umount sda5, sda9 and sda1 again"
umount /dev/sda5
umount /dev/sda9
sleep 1
umount /dev/sda1

echo "Start to copy files to bank 2"
mkdir /mnt/ssd2
mount /dev/sda2 /mnt/ssd2

cd /mnt/ssd2
mkdir boot
mkdir home
mount /dev/sda6 boot
mount /dev/sda9 home

mkdir boot/ucnt
cp /mnt/usb/T600/fp01.bin boot/ucnt/
#shall be remove
cp /mnt/usb/T600/fp01.bin .

tar zxf ${file_name} -C /mnt/ssd2
sync
sleep 1

if [ "${is_pgm}" = "1" ]
then
    found=0

    file_list=$(ls /mnt/ssd2/*.tar.gz)
    for real_file_name in ${file_list}
    do
        echo "find a file "${real_file_name}
        found=1
        break;
    done

    if [ "${found}" != "1" ]
    then
        echo "There is no any *.tar.gz file in thg *.PGM."
        echo "Stop procedure"
        exit 0
    fi

    tar zxf ${real_file_name} -C .
    sync
    sleep 1
fi

cd boot
ln -f -s uImage-3.12.19-rt30-4.0_2+g6619b8b uImage-t600.bin
ln -f -s t600.dtb uImage-t600.dtb
cd ..
sync

echo "Umount sda6, sda9 and sda2 "
cd ..
umount /dev/sda6
umount /dev/sda9
sleep 1
umount /dev/sda2

e2fsck /dev/sda2
e2fsck /dev/sda6
e2fsck /dev/sda9

echo "Mount sda6, sda9 and sda2 again to create the journaling"
mount /dev/sda2 /mnt/ssd2
mount /dev/sda6 /mnt/ssd2/boot
mount /dev/sda9 /mnt/ssd2/home

echo "Umount sda6, sda9 and sda2 again"
umount /dev/sda6
umount /dev/sda9
sleep 1
umount /dev/sda2

echo "Done"
