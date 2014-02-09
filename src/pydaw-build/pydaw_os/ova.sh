PYDAW_VERSION=pydaw4
PYDAW_NAME=$PYDAW_VERSION-$(cat ../../pydaw4-version.txt)
PYDAW_OVA_NAME=$PYDAW_NAME-virtualbox.ova
PYDAW_WIN_NAME=$PYDAW_NAME-windows.zip
PYDAW_MAC_NAME=$PYDAW_NAME-mac.zip

if [ -f $PYDAW_OVA_NAME ]; then
	rm -f $PYDAW_OVA_NAME
fi

rm *.zip

vboxmanage export $PYDAW_VERSION -o $PYDAW_OVA_NAME
zip $PYDAW_MAC_NAME $PYDAW_OVA_NAME Virtual*dmg readme_windows_and_mac.txt
zip $PYDAW_WIN_NAME $PYDAW_OVA_NAME Virtual*exe readme_windows_and_mac.txt
