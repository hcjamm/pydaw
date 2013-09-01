cp /usr/lib/pydaw3/mixxx/mixxx-launcher.py /usr/bin
apt-get clean
rm -rf /tmp/* ~/.bash_history
#rm /etc/hosts
#rm /etc/resolv.conf
#rm /var/lib/dbus/machine-id
#rm /sbin/initctl
#dpkg-divert --rename --remove /sbin/initctl
umount /proc || umount -lf /proc
umount /sys
umount /dev/pts

echo
echo "#######Now run:#######"
echo "exit"
echo "sudo umount -lf edit/dev"
echo "sudo ./finish.sh"
echo
