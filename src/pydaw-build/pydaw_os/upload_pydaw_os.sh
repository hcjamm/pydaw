PYDAW_VERSION=$(cat ../../major-version.txt)
PYDAW_ISO_NAME=pydaw-os-$(cat ../../minor-version.txt).iso
PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/pydaw_os

echo "scp'ing $PYDAW_ISO_NAME $PYDAW_DIR"

scp -l 3000 $PYDAW_ISO_NAME $PYDAW_DIR

