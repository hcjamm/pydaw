# Pretty much copied and pasted from here:
# http://blog.willhaley.com/create-a-custom-debian-live-environment/

#check for root
if [ "$(id -u)" != "0" ]; then
   echo "Error:  This script must be run as root, use su" 1>&2
   exit 1
fi


apt-get install squashfs-tools genisoimage debootstrap syslinux memtest86+ rsync

if [ ! -d chroot ]; then
	debootstrap --arch=amd64 --variant=minbase wheezy chroot http://ftp.us.debian.org/debian/
else
	echo "chroot directory exists, not running debootstrap"	
fi

mount -o bind /dev chroot/dev && cp /etc/resolv.conf chroot/etc/resolv.conf

cp pydaw-live.sh chroot/etc/profile.d/
pydaw_live_script=chroot/etc/profile.d/pydaw-live.sh
chmod +x "$pydaw_live_script"
chmod 755 "$pydaw_live_script"
cp quit.sh chroot/root/
cp setup.sh chroot/root/
chmod 755 chroot/root/quit.sh chroot/root/setup.sh

echo
echo
echo "##Chroot'ing into the live DVD, now you can can update and install software##"
echo "##Run##"
echo "cd /root"
echo "./setup.sh"
echo "##To prepare the environment##"
echo "##Run ./quit.sh in /root to cleanup and exit##"
echo

chroot chroot

