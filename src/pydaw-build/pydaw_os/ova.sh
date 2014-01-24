PYDAW_VERSION=pydaw4
PYDAW_OVA_NAME=$PYDAW_VERSION-$(cat ../../pydaw4-version.txt).ova

if [ -f $PYDAW_OVA_NAME ]; then
	rm -f $PYDAW_OVA_NAME
fi

vboxmanage export $PYDAW_VERSION -o $PYDAW_OVA_NAME

