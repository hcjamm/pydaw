# Pretty much copied and pasted from here:
# http://blog.willhaley.com/create-a-custom-debian-live-environment/

#check for root
if [ "$(id -u)" != "0" ]; then
   echo "Error:  This script must be run as root, use su" 1>&2
   exit 1
fi

if [ -d image ]; then
	rm -rf image
fi

mkdir -p image/{live,isolinux}
mksquashfs chroot image/live/filesystem.squashfs -e boot
cp chroot/boot/vmlinuz-* image/live/vmlinuz1 && 
cp chroot/boot/initrd.img* image/live/initrd1
cp isolinux.cfg image/isolinux

cp /usr/lib/syslinux/isolinux.bin image/isolinux/ && 
cp /usr/lib/syslinux/menu.c32 image/isolinux/ && 
cp /usr/lib/syslinux/hdt.c32 image/isolinux/ && 
cp /boot/memtest86+.bin image/live/memtest

#cp /usr/lib/syslinux/menu.c32 image/isolinux && 
#cp /usr/lib/syslinux/hdt.c32 image/isolinux 

cd image

mv isolinux syslinux
cd syslinux
mv isolinux.bin syslinux.bin
mv isolinux.cfg syslinux.cfg

cd ..

genisoimage -rational-rock -volid "Debian Live" -cache-inodes -joliet -full-iso9660-filenames -b syslinux/syslinux.bin -c syslinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -output ../pydawbian.iso .

cd ..

