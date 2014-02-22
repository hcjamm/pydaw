git pull
./install_deb.sh
PYDAW_VERSION=$(cat major-version.txt)
PYDAW_RELEASE=$(cat minor-version.txt)

PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/linux

scp pydaw-build/$PYDAW_VERSION-$PYDAW_RELEASE*deb $PYDAW_DIR

