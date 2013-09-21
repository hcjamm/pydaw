git pull
PYDAW_VERSION=pydaw3-$(cat src/pydaw3-version.txt)
cp -r src $PYDAW_VERSION
tar czf $PYDAW_VERSION-source-code.tar.gz $PYDAW_VERSION
rm -rf $PYDAW_VERSION

