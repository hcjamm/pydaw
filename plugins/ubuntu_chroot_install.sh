#  The script for turning a chroot'ed Ubuntu or Linux Mint into a PyDAW Live DVD/USB image automagically
#
#  in Linux Mint, this is done by:
#
#    sudo apt-get install mintconstructor
#    sudo /usr/lib/linuxmint/mintConstructor/mintConstructor.py

#This script assumes that the following have already happened:
#cd ~
#apt-get -y install git
#git clone https://github.com/j3ffhubb/audiocode.git
#cd audiocode/plugins
git pull
apt-get -y install audacity  #Because for some reason Linux Mint refuses to install it as a dependency of PyDAW

#This next line will require user interaction, so it is not yet suitable for continuous integration style
#nightly builds.  OTOH, I have no reason to want to provide nightly builds, and the questions being asked
#probably should be answered by a human being anyways, so it's all good...
perl deb.pl

#So, assuming you answered 'yes' and 'yes' to "install dependencies?" and "install PyDAW now?", you should
#now have a functional PyDAW installation with all dependencies, and QJackCtl and Audacity, until I eventually
#deprecate those in favor of native PyDAW equivalents...

#These were the cleanup lines recommended by the Linux Mint folks here:
#  http://community.linuxmint.com/tutorial/view/918
#Some of the commands might be obsolete now

echo -e "\n\n\n******PyDAW installation complete, cleaning up now...******\n\n\n"

# At login, create the DNS fix startup script so that internet will  work properly in Mint...
# Also create desktop icons for live users...
pydaw_live_script=/etc/profile.d/pydaw-live.sh
cp distro/mint/pydaw-live.sh "$pydaw_live_script"
chmod +x "$pydaw_live_script"
chmod 755 "$pydaw_live_script"

#TODO:  Remove some of the no-longer-needed build dependencies to conserve space now???

aptitude purge ~c
aptitude unmarkauto ~M
apt-get clean
rm -rf /var/cache/debconf/*.dat-old
rm -rf /var/lib/aptitude/*.old
rm -rf /var/lib/dpkg/*-old
rm -rf /var/cache/apt/*.bin
updatedb
history -c
rm /root/.bash_history
rm /root/.nano_history
history -c

