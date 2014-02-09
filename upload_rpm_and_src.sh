PYDAW_VERSION=pydaw4
PYDAW_VERSION=$(cat src/pydaw4-version.txt)
PYDAW_DIR=pydawteam@frs.sourceforge.net:/home/frs/project/libmodsynth/$PYDAW_VERSION/linux

scp ~/rpmbuild/RPMS/*/*$PYDAW_VERSION*rpm ~/rpmbuild/SRPMS/*$PYDAW_VERSION*rpm $PYDAW_DIR

