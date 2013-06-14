mount -t proc none /proc
mount -t sysfs none /sys
mount -t devpts none /dev/pts
export HOME=/root
export LC_ALL=C

# dbus-uuidgen > /var/lib/dbus/machine-id
# dpkg-divert --local --rename --add /sbin/initctl
# ln -s /bin/true /sbin/initctl

apt-get update
#apt-get upgrade -y
#This fails to install when a dependency of another package,
#hence we install manually first
apt-get install -y audacity
apt-get install -y mixxx
cp /usr/lib/pydaw3/mixxx/mixxx-launcher.py /usr/bin
#Here either wget a .deb, or (preferably) have deb.pl move it here when building...

#dpkg -i pydaw*.deb
#Due the WTF normally encountered when doing this, it's often necessary to run:
#apt-get install -f
