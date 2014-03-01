rm -f *.iso

#./copy_rpms.py
#sudo ./build.py

PYDAW_VERSION=$(cat ../../major-version.txt)
PYDAW_RELEASE=$(cat ../../minor-version.txt)
PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/pydaw_os
PYDAW_ISO=PyDAW-OS-Fedora-KDE-$PYDAW_RELEASE.iso
echo "scp'ing $PYDAW_ISO $PYDAW_DIR"
scp -l 3000  $PYDAW_ISO $PYDAW_DIR

