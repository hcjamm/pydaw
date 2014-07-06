git pull
PYDAW_VERSION=$(cat src/major-version.txt)-$(cat src/minor-version.txt)
cp -r src $PYDAW_VERSION
tar czf ${PYDAW_VERSION}.tar.gz $PYDAW_VERSION
rm -rf $PYDAW_VERSION

