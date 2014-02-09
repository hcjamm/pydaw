PYDAW_VERSION=pydaw4
PYDAW_RELEASE=$(cat ../../pydaw4-version.txt)
PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/pydaw_os

scp $PYDAW_VERSION*$PYDAW_RELEASE*iso $PYDAW_DIR

