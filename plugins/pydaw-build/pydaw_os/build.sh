# Pretty much copied and pasted from here with very little in the way of fixes:
# https://help.ubuntu.com/community/LiveCDCustomization
# and unlike other crap-tastic failures like 
# UCK, Reconstructor.Engine, etc...  this actually seems to work with consistency...
# I may even spin it into a PyQt GUI application to share with the world, because seriously,
# software development is hard, but not as hard as you people make it look...
# /Frustrated rant after having to write my own solution to a fairly simple and universal problem

sudo apt-get install squashfs-tools genisoimage

if [ $# != 1 ]; then
	echo "usage:  ./build.sh [input .iso file]"
fi

ISO_NAME=$1

if [ ! -f $ISO_NAME ]; then
	echo "You must place $ISO_NAME in this directory before continuing"
	exit 1
fi

if [ ! -d extract-cd ]; then
	mkdir mnt
	sudo mount -o loop $ISO_NAME mnt
	mkdir extract-cd
	sudo rsync --exclude=/casper/filesystem.squashfs -a mnt/ extract-cd
	sudo unsquashfs mnt/casper/filesystem.squashfs
	sudo mv squashfs-root edit
	sudo umount -lf mnt
else
	echo "extract-cd directory exists, not extracting from live DVD"
fi

sudo cp /etc/resolv.conf edit/etc/

sudo cp /etc/apt/sources.list edit/etc/apt/

sudo cp pydaw-live.sh edit/etc/profile.d/
pydaw_live_script=edit/etc/profile.d/pydaw-live.sh
sudo chmod +x "$pydaw_live_script"
sudo chmod 755 "$pydaw_live_script"
sudo cp quit.sh edit/root/
sudo cp setup.sh edit/root/
sudo chmod 755 edit/root/quit.sh edit/root/setup.sh
sudo mount --bind /dev/ edit/dev

echo
echo
echo "##Chroot'ing into the live DVD, now you can can update and install software##"
echo "##Run##"
echo "cd /root"
echo "./setup.sh"
echo "##To prepare the environment##"
echo "##Run ./quit.sh in /root to cleanup and exit##"
echo

sudo chroot edit

