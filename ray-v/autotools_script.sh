aclocal
libtoolize --force --copy
autoheader
automake --add-missing --foreign
autoconf
./configure
make
sudo checkinstall
