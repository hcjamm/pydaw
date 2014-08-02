#check for root
if [ "$(id -u)" != "0" ]; then
   echo "Error:  This script must be run as root, use sudo" 1>&2
   exit 1
fi

umount -lf edit/dev

#Regenerate the manifest
chmod +w extract-cd/casper/filesystem.manifest
chroot edit dpkg-query -W --showformat='${Package} ${Version}\n' > extract-cd/casper/filesystem.manifest
cp extract-cd/casper/filesystem.manifest extract-cd/casper/filesystem.manifest-desktop
sed -i '/ubiquity/d' extract-cd/casper/filesystem.manifest-desktop
sed -i '/casper/d' extract-cd/casper/filesystem.manifest-desktop

#Compress the filesystem
rm extract-cd/casper/filesystem.squashfs  #<- Didn't exist?
mksquashfs edit extract-cd/casper/filesystem.squashfs  #-comp xz -e edit/boot

#Update the filesystem size
printf $(du -sx --block-size=1 edit | cut -f1) > extract-cd/casper/filesystem.size
#Set an image name
#nano extract-cd/README.diskdefines
echo "pydaw_os" > extract-cd/README.diskdefines

# Regenerate md5sums
cd extract-cd
rm md5sum.txt
find -type f -print0 | xargs -0 md5sum | grep -v isolinux/boot.cat | tee md5sum.txt

PYDAW_ISO_NAME=pydaw-os-$(cat ../../../minor-version.txt).iso

#Create the .iso image
mkisofs -D -r -V "$IMAGE_NAME" -cache-inodes -J -l -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -o ../$PYDAW_ISO_NAME .

cd ..
md5sum $PYDAW_ISO_NAME > md5sum.txt
