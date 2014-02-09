PYDAW_VERSION=pydaw4
PYDAW_ISO_NAME=pydaw-os-$(cat ../../pydaw4-version.txt).iso
PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/pydaw_os

scp $PYDAW_ISO_NAME $PYDAW_DIR

