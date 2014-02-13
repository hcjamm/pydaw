PYDAW_VERSION=pydaw4
PYDAW_NAME=$PYDAW_VERSION-$(cat ../../pydaw4-version.txt)
PYDAW_WIN_NAME=$PYDAW_NAME-windows.zip
PYDAW_MAC_NAME=$PYDAW_NAME-mac.zip

PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION

scp $PYDAW_WIN_NAME $PYDAW_DIR/windows
scp $PYDAW_MAC_NAME $PYDAW_DIR/mac

