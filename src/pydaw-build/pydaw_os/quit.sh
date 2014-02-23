PYDAW_VERSION=$(cat ../../major-version.txt)

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
rm pydaw*.deb

echo
echo "#######Now run the following commands:#######"
echo "exit"
echo "sudo ./finish.sh"
echo
