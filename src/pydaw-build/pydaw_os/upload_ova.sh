rm -f *.ova

./ova.sh

PYDAW_VERSION=$(cat ../../major-version.txt)
PYDAW_NAME=$PYDAW_VERSION-$(cat ../../minor-version.txt)
PYDAW_WIN_NAME=$PYDAW_NAME-windows.zip
PYDAW_MAC_NAME=$PYDAW_NAME-mac.zip

PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION

$WIN_DIR = $PYDAW_DIR/windows
$MAC_DIR = $PYDAW_DIR/mac

echo "scp'ing $PYDAW_WIN_NAME $WIN_DIR"

scp -l 3000 $PYDAW_WIN_NAME $WIN_DIR

echo "scp'ing $PYDAW_MAC_NAME $MAC_DIR"

scp -l 3000 $PYDAW_MAC_NAME $MAC_DIR

