git pull
PYDAW_VERSION=pydaw4-$(sed 's/-/\./g' src/pydaw4-version.txt)
cp -r src $PYDAW_VERSION
tar czf $PYDAW_VERSION-source-code.tar.gz $PYDAW_VERSION
rm -rf $PYDAW_VERSION

