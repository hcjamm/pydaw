How to build:

Ubuntu/Debian distros:

cd [pydaw dir]/src
./ubuntu_deps.sh
make deps
./install_deb.sh

Fedora based distros:

cd [pydaw dir]/src
./fedora_deps.sh
cd ..
./rpm.py
cd ~/rpmbuild/RPMS/[your arch]
sudo rpm -ivh pydaw*rpm

All others:

# figure out the dependencies based on the Fedora or Ubuntu dependencies

cd [pydaw dir]/src
make
make install  # You can specify DESTDIR or PREFIX if packaging, the result is relocatable
