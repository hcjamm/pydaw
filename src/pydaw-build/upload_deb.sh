PYDAW_VERSION=pydaw4
PYDAW_RELEASE=$(cat ../pydaw4-version.txt)

PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/linux

scp $PYDAW_VERSION-$PYDAW_RELEASE*deb $PYDAW_DIR

