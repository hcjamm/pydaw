cp /usr/lib/pydaw3/mixxx/mixxx-launcher.py /usr/bin
rm ~/.bash_history
rm -f /var/lib/dbus/machine-id && 
apt-get clean && 
rm -rf /tmp/* && 
rm /etc/resolv.conf && 
umount -lf /proc && 
umount -lf /sys && 
umount -lf /dev/pts

echo "#######Now run as root:#######"
echo "exit"
echo "umount -lf chroot/dev"
echo "./finish.sh"
echo
