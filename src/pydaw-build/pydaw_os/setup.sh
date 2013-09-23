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
apt-get install -y audacity midori

apt-get install -y mixxx

apt-get remove -y jockey* brasero* gwibber* libreoffice-* thunderbird* usb-creator* aisleriot mahjongg transmission* pulseaudio firefox*

apt-get autoremove -y

#Install the desired PyDAW .deb with
#dpkg -i pydaw*.deb

#Because dpkg can't resolve dependencies, you will have to run this if all 
#dependencies were'nt already installed
#apt-get install -f
