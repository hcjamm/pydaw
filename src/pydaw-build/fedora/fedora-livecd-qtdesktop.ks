# fedora-livecd-qtdesktop.ks
#
# v.4 - all inclusive
#
# Description:
# - Fedora Live Spin with the QtDesktop Environment
#
# Maintainer(s):
# - TI_Eugene <ti.eugene@gmail.com>
#
# Requirements:
# - RAM: LiveCD - ..., Installation - 256MB, Installed: ...
# - CD:  RAM: HDD:
# - HDD:
#

%include /usr/share/spin-kickstarts/fedora-live-base.ks
%include /usr/share/spin-kickstarts/fedora-live-minimization.ks

# because of lightdm
selinux --permissive

%packages

# +QtDesktop:
clementine
goldendict
juffed-plugins
nomacs
pcmanfm-qt
qastools
qbittorrent
qlipper
qpdfview
qterminal
qtparted
quiterss
qupzilla
qutim
qxkb
screengrab
#smplayer
speedcrunch
#vlc
razorqt
lightdm-razorqt

%end

%post
# 1. create /etc/sysconfig/desktop (required for installation)
cat > /etc/sysconfig/desktop <<EOF
PREFERRED=/usr/bin/startrazor
DISPLAYMANAGER=/usr/sbin/lightdm
EOF

# 2. gtk icon theme (requied for anaconda)
cat > /etc/gtk-3.0/settings.ini <<EOF
[Settings]
gtk-icon-theme-name=gnome
EOF

# 3. autologin
/usr/libexec/lightdm/lightdm-set-defaults --autologin liveuser
ln -s razor.desktop /usr/share/xsessions/default.desktop

# 4. Show "Install to hard disk" in menu
sed -i -e 's/NoDisplay=true/NoDisplay=false/g' /usr/share/applications/liveinst.desktop

%end
