#  The script for turning a chroot'ed Ubuntu into a PyDAW Live DVD/USB image automagically
#
#  Currently this is being done with Reconstructor:
#       http://www.reconstructor.org
#  

#This script assumes that the following have already happened:
#cd ~
#apt-get -y install git
#git clone https://github.com/j3ffhubb/audiocode.git
#cd audiocode/plugins
git pull
apt-get -y install audacity  #Because for some reason it refuses to install it as a dependency of PyDAW

#This next line will require user interaction, so it is not yet suitable for continuous integration style
#nightly builds.  OTOH, I have no reason to want to provide nightly builds, and the questions being asked
#probably should be answered by a human being anyways, so it's all good...
perl deb.pl

#So, assuming you answered 'yes' and 'yes' to "install dependencies?" and "install PyDAW now?", you should
#now have a functional PyDAW installation with all dependencies, and QJackCtl and Audacity, until I eventually
#deprecate those in favor of native PyDAW equivalents...

cp ./sources.list /etc/apt/sources.list

# create desktop icons for live users...
pydaw_live_script=/etc/profile.d/pydaw-live.sh
cp ./pydaw-live.sh "$pydaw_live_script"
chmod +x "$pydaw_live_script"
chmod 755 "$pydaw_live_script"

echo -e "\n\n\n******PyDAW installation complete, cleaning up now...******\n\n\n"

#These were the cleanup lines recommended by the Linux Mint folks here:
#  http://community.linuxmint.com/tutorial/view/918
#Some of the commands might be obsolete now, because they error out with a "I don't know WTF you're talking about" error.

#TODO:  Remove some of the no-longer-needed build dependencies to conserve space now???

apt-get purge
apt-get autoremove
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

