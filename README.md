#Deprecation notice

PyDAW is being superceded by my new project here:

https://github.com/j3ffhubb/musikernel

The new project is still under heavy development and not yet recommended for any use other than evaluation.

#How to build:

#Ubuntu/Debian distros:

cd [pydaw src dir]/src

./ubuntu_deps.sh

make deps

make deb

cd ../pydaw-build

sudo dpkg -i pydaw[your_version].deb

#Fedora based distros:

cd [pydaw src dir]/src

./fedora_deps.sh

make rpm

cd ~/rpmbuild/RPMS/[your arch]

sudo rpm -e pydaw4  # You can skip this if PyDAW is not already installed

sudo yum localinstall pydaw[version number].rpm

#All others:

 # [figure out the dependencies based on the Fedora or Ubuntu dependencies]

cd [pydaw src dir]/src

make

 # You can specify DESTDIR or PREFIX if packaging, the result is fully relocatable

make install

