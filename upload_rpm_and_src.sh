./rpm.py

PYDAW_VERSION=$(cat src/major-version.txt)
PYDAW_RELEASE=$(cat src/minor-version.txt)
PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/linux

scp ~/rpmbuild/RPMS/*/*$PYDAW_RELEASE*rpm ~/rpmbuild/SRPMS/*$PYDAW_RELEASE*rpm *$PYDAW_VERSION-$PYDAW_RELEASE-source-code.tar.gz $PYDAW_DIR

