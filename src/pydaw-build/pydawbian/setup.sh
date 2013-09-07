mount none -t proc /proc && 
mount none -t sysfs /sys && 
mount none -t devpts /dev/pts && 
export HOME=/root && 
export LC_ALL=C && 
apt-get install dialog dbus --yes && 
dbus-uuidgen > /var/lib/dbus/machine-id && 
apt-get update --yes
echo "debian-live" > /etc/hostname

apt-get install --yes \
linux-image-amd64 live-boot network-manager net-tools \
wireless-tools wpagui tcpdump wget openssh-client \
blackbox xserver-xorg-core xserver-xorg xinit xterm \
pciutils usbutils gparted ntfsprogs hfsprogs rsync \
dosfstools syslinux partclone nano pv rtorrent iceweasel \
chntpw lxde audacity mixxx iceweasel nano makepasswd

id -u pydaw &>/dev/null || useradd -p $(echo "pydaw" | makepasswd) pydaw
