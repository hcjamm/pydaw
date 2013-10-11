#!/usr/bin/env python3
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import os, subprocess, sys

f_base_dir = os.path.dirname(os.path.abspath(__file__))

print(f_base_dir)

os.chdir(f_base_dir)

def pydaw_read_file_text(a_file):
    f_handle = open(str(a_file))
    f_result = f_handle.read()
    f_handle.close()
    return f_result

def pydaw_write_file_text(a_file, a_text):
    f_handle = open(str(a_file), "w")
    f_handle.write(str(a_text))
    f_handle.close()

global_pydaw_version_string = "pydaw3"

f_build_cmd = \
'make clean && make && make DESTDIR="%s/pydaw-build/debian" install' % (f_base_dir,)
f_version_file = "%s/%s-version.txt" % (f_base_dir, global_pydaw_version_string)

f_short_name = global_pydaw_version_string
f_arch = subprocess.getoutput("dpkg --print-architecture").strip()

os.system("rm -rf pydaw-build/debian/usr")
os.system("mkdir -p pydaw-build/debian/usr")
os.system('find ./pydaw-build/debian -type f -name *~  -exec rm -f {} \\;')
os.system('find ./pydaw-build/debian -type f -name *.pyc  -exec rm -f {} \\;')
os.system('find ./pydaw-build/debian -type f -name core  -exec rm -f {} \\;')

f_makefile_exit_code = os.system(f_build_cmd)

if f_makefile_exit_code != 0:
    print(("Makefile exited abnormally with exit code %s, see output for error messages." % (f_makefile_exit_code,)))
    print("If the build failed while compiling Portaudio, you should try this workaround:")
    print("cd pydaw/portaudio")
    print("./configure --with-jack=no --with-oss=no && make clean && make")
    print("...and then retry running deb.py")
    sys.exit(f_makefile_exit_code)

f_version = pydaw_read_file_text(f_version_file).strip()

if not "--default-version" in sys.argv:
    f_version_new = input("""Please enter the version number of this release.
    The format should be something like:  1.1.3-1 or 12.04-1
    Hit enter to accept the auto-generated default version number:  %s
    [version number]: """ % (f_version,))
    if f_version_new.strip() != "":
        f_version = f_version_new.strip()
        pydaw_write_file_text(f_version_file, f_version)

f_size = subprocess.getoutput('du -s "%s/pydaw-build/debian/usr"' % (f_base_dir,))
f_size = f_size.replace("\t", " ")
f_size = f_size.split(" ")[0].strip()

f_debian_control = """
Package: %s
Priority: extra
Section: sound
Installed-Size: %s
Maintainer: PyDAW Team <pydawteam@users.sf.net>
Architecture: %s
Version: %s
Depends: libasound2-dev, liblo-dev, libsndfile1-dev, libportmidi-dev, python3-pyqt4, audacity, python3, python3-scipy, python3-numpy, libsamplerate0-dev, libfftw3-dev
Provides: %s
Conflicts:
Replaces:
Description: A digital audio workstation with a full suite of instrument and effects plugins.
 PyDAW is a full featured audio and MIDI sequencer with a suite of high quality instrument and effects plugins.
""" % (f_short_name, f_size, f_arch, f_version, f_short_name)

f_debian_dir = "%s/pydaw-build/debian/DEBIAN" % (f_base_dir,)

if not os.path.isdir(f_debian_dir):
    os.makedirs(f_debian_dir)

f_debian_control_path = "%s/control" % (f_debian_dir,)

pydaw_write_file_text(f_debian_control_path, f_debian_control)

os.system('chmod 755 "%s"' % (f_debian_control_path,))
os.system("cd pydaw-build/debian; find . -type f ! -regex '.*\.hg.*' ! -regex '.*?debian-binary.*' ! -regex '.*?DEBIAN.*' -printf '%P ' | xargs md5sum > DEBIAN/md5sums")
os.system("chmod -R 755 pydaw-build/debian/usr ; chmod 644 pydaw-build/debian/DEBIAN/md5sums")

f_build_suffix_file = '%s/build-suffix.txt' % (f_base_dir,)
if os.path.exists(f_build_suffix_file):
    f_build_suffix = pydaw_read_file_text(f_build_suffix_file)
else:
    f_build_suffix = input("""You may enter an optional build suffix.  Usually this will be the
operating system you are compiling for on this machine, for example: ubuntu1210

Please enter a build suffix, or hit 'enter' to leave blank: """).strip()
    if f_build_suffix != "": f_build_suffix = "-" + f_build_suffix
    pydaw_write_file_text(f_build_suffix_file, f_build_suffix)

f_package_name = "%s-%s-%s%s.deb" % (f_short_name, f_version, f_arch, f_build_suffix)

os.system('rm -f "%s"/pydaw-build/pydaw*.deb' % (f_base_dir,))

if os.geteuid() == 0:
    f_eng_bin = '"%s"/pydaw-build/debian/usr/bin/%s-engine' % (f_base_dir, global_pydaw_version_string)
    os.system('chown root %s' % (f_eng_bin,))
    os.system('chmod 4755 %s' % (f_eng_bin,))
    os.system('cd "%s"/pydaw-build && dpkg-deb --build debian && mv debian.deb "%s"' %
        (f_base_dir, f_package_name))
else:
    print("Not running as root, using fakeroot to build Debian package.")
    os.system('cd "%s"/pydaw-build && fakeroot dpkg-deb --build debian && mv debian.deb "%s"' %
        (f_base_dir, f_package_name))

if not "--keep" in sys.argv:
    os.system('rm -rf "%s/pydaw-build/debian"' % (f_base_dir,))

print("Finished. run ./install_deb.sh to install the package")
