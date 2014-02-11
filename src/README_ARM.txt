Compiling for ARM:
(these instructions are not suitable for creating general purpose ARM packages,
but you can use them to compile PyDAW for a specific development board like
the ODROID, or the ARM Chromebook)

# Install all required dependencies
./ubuntu_deps.sh

# or

./fedora_deps.sh

make native
make install

# or

make PREFIX=$(your prefix) DESTDIR=$(where you want to install it) install

# ^PyDAW is fully relocatable, you can do a root-less install to any folder
# you'd like, and you can even move the folder later.

