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

import os
import sys

os.chdir(os.path.dirname(os.path.abspath(__file__)))
os.system("./src.sh")

with open("src/major-version.txt") as f_file:
    global_pydaw_version_string = f_file.read().strip()

with open("src/minor-version.txt") as f_file:
    global_pydaw_version_num = f_file.read().strip()

global_version_fedora = global_pydaw_version_num.replace("-", ".")
global_pydaw_package_name = "{}-{}".format(
    global_pydaw_version_string, global_version_fedora)

global_home = os.path.expanduser("~")

if not os.path.isdir("{}/rpmbuild".format(global_home)):
    os.system("rpmdev-setuptree")

global_specs_dir = "{}/rpmbuild/SPECS/".format(global_home)
global_sources_dir = "{}/rpmbuild/SOURCES/".format(global_home)

global_tarball_name = "{}.tar.gz".format(global_pydaw_package_name)
global_tarball_url = ("https://github.com/j3ffhubb/pydaw/archive"
    "/{}".format(global_tarball_name))

os.system('cp "{}" "{}"'.format(global_tarball_name, global_sources_dir))

global_spec_file = "{}.spec".format(global_pydaw_version_string,)

global_rpmmacros_file = open("{}/.rpmmacros".format(global_home), "r")
global_macro_text = global_rpmmacros_file.read()

# Creating separate debug packages screw up the inclusion of both debug,
# non-debug and setuid binaries, so we need to force rpmbuild not to strip
if not "%debug_package %{nil}" in global_macro_text:
    global_rpmmacros_file.close()
    global_rpmmacros_file = open("{}/.rpmmacros".format(global_home), "a")
    global_rpmmacros_file.write("\n%debug_package %{nil}\n")
else:
    global_macro_text = None

global_rpmmacros_file.close()

if "--native" in sys.argv:
    f_native = "native"
else:
    f_native = ""

f_spec_template = \
"""
Name:           {0}
Version:        {1}

Release:        1%{{?dist}}
Summary:        A digital audio workstation with a full suite of instrument and effects plugins.

License:        GPLv3
URL:            http://github.com/j3ffhubb/pydaw/
Source0:        {2}

Requires:      python3-PyQt4 gcc alsa-lib-devel liblo-devel \
libsndfile-devel gcc-c++ git python3-numpy python3-scipy \
fftw-devel portmidi-devel libsamplerate-devel python3-devel vorbis-tools

%description
PyDAW is a full featured audio and MIDI sequencer with a suite of high quality
instrument and effects plugins.

%prep
%setup -q

%build
make {3}

%install
export DONT_STRIP=1
rm -rf $RPM_BUILD_ROOT
%make_install

%post
%preun

%files

%defattr(644, root, root)

%attr(4755, root, root) /usr/bin/{0}-engine

%attr(755, root, root) /usr/bin/{0}
%attr(755, root, root) /usr/bin/{0}_render
%attr(755, root, root) /usr/bin/{0}_render-dbg
%attr(755, root, root) /usr/bin/{0}-engine-dbg
%attr(755, root, root) /usr/bin/{0}-engine-no-hw
%attr(755, root, root) /usr/bin/{0}-engine-no-root
%attr(755, root, root) /usr/bin/{0}-project-recover
%attr(755, root, root) /usr/lib/{0}/pydaw/python/libpydaw/pydaw_paulstretch.py
%attr(755, root, root) /usr/lib/{0}/pydaw/python/pydaw.py
%attr(755, root, root) /usr/lib/{0}/rubberband/bin/rubberband
%attr(755, root, root) /usr/lib/{0}/sbsms/bin/sbsms

/usr/lib/{0}/presets/MODULEX.pypresets
/usr/lib/{0}/presets/RAYV.pypresets
/usr/lib/{0}/presets/WAYV.pypresets
/usr/lib/{0}/pydaw/python/libpydaw/__init__.py
/usr/lib/{0}/pydaw/python/libpydaw/liblo.cpython-33m.so
/usr/lib/{0}/pydaw/python/libpydaw/libportaudio.so
/usr/lib/{0}/pydaw/python/libpydaw/midicomp
/usr/lib/{0}/pydaw/python/libpydaw/portaudio.py
/usr/lib/{0}/pydaw/python/libpydaw/portmidi.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_device_dialog.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_gradients.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_history.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_osc.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_ports.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_project.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_util.py
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_widgets.py
/usr/lib/{0}/pydaw/python/libpydaw/staging.py
/usr/lib/{0}/pydaw/python/libpydaw/super_formant_maker.py
/usr/lib/{0}/pydaw/python/libpydaw/translate.py
/usr/lib/{0}/major-version.txt
/usr/lib/{0}/minor-version.txt
/usr/lib/{0}/rubberband/include/rubberband/RubberBandStretcher.h
/usr/lib/{0}/rubberband/include/rubberband/rubberband-c.h
/usr/lib/{0}/rubberband/lib/librubberband.a
/usr/lib/{0}/rubberband/lib/librubberband.so
/usr/lib/{0}/rubberband/lib/librubberband.so.2
/usr/lib/{0}/rubberband/lib/librubberband.so.2.1.0
/usr/lib/{0}/rubberband/lib/pkgconfig/rubberband.pc
/usr/lib/{0}/themes/default/drop-down.png
/usr/lib/{0}/themes/default/euphoria.png
/usr/lib/{0}/themes/default/h-fader.png
/usr/lib/{0}/themes/default/mute-off.png
/usr/lib/{0}/themes/default/mute-on.png
/usr/lib/{0}/themes/default/play-off.png
/usr/lib/{0}/themes/default/play-on.png
/usr/lib/{0}/themes/default/pydaw-knob.png
/usr/lib/{0}/themes/default/rayv.png
/usr/lib/{0}/themes/default/rec-off.png
/usr/lib/{0}/themes/default/rec-on.png
/usr/lib/{0}/themes/default/record-off.png
/usr/lib/{0}/themes/default/record-on.png
/usr/lib/{0}/themes/default/solo-off.png
/usr/lib/{0}/themes/default/solo-on.png
/usr/lib/{0}/themes/default/spinbox-down.png
/usr/lib/{0}/themes/default/spinbox-up.png
/usr/lib/{0}/themes/default/stop-off.png
/usr/lib/{0}/themes/default/stop-on.png
/usr/lib/{0}/themes/default/default.pytheme
/usr/lib/{0}/themes/default/v-fader.png
/usr/share/applications/{0}.desktop
/usr/share/doc/{0}/copyright
/usr/share/doc/{0}/readme.txt
/usr/share/pixmaps/{0}.png
#/usr/share/locale/pt_PT/LC_MESSAGES/{0}.mo
#/usr/share/locale/de/LC_MESSAGES/{0}.mo
#/usr/share/locale/fr/LC_MESSAGES/{0}.mo
/usr/lib/{0}/pydaw/python/wavefile/__init__.py
/usr/lib/{0}/pydaw/python/wavefile/libsndfile.py
/usr/lib/{0}/pydaw/python/wavefile/wavefile.py


/usr/lib/{0}/pydaw/python/libpydaw/__init__.pyc
/usr/lib/{0}/pydaw/python/libpydaw/__init__.pyo
/usr/lib/{0}/pydaw/python/libpydaw/portaudio.pyc
/usr/lib/{0}/pydaw/python/libpydaw/portaudio.pyo
/usr/lib/{0}/pydaw/python/libpydaw/portmidi.pyc
/usr/lib/{0}/pydaw/python/libpydaw/portmidi.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_device_dialog.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_device_dialog.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_gradients.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_gradients.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_history.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_history.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_osc.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_osc.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_paulstretch.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_paulstretch.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_ports.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_ports.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_project.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_project.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_util.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_util.pyo
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_widgets.pyc
/usr/lib/{0}/pydaw/python/libpydaw/pydaw_widgets.pyo
/usr/lib/{0}/pydaw/python/libpydaw/staging.pyc
/usr/lib/{0}/pydaw/python/libpydaw/staging.pyo
/usr/lib/{0}/pydaw/python/libpydaw/super_formant_maker.pyc
/usr/lib/{0}/pydaw/python/libpydaw/super_formant_maker.pyo
/usr/lib/{0}/pydaw/python/pydaw.pyc
/usr/lib/{0}/pydaw/python/pydaw.pyo
/usr/lib/{0}/pydaw/python/libpydaw/translate.pyc
/usr/lib/{0}/pydaw/python/libpydaw/translate.pyo
/usr/lib/{0}/pydaw/python/wavefile/__init__.pyc
/usr/lib/{0}/pydaw/python/wavefile/__init__.pyo
/usr/lib/{0}/pydaw/python/wavefile/libsndfile.pyc
/usr/lib/{0}/pydaw/python/wavefile/libsndfile.pyo
/usr/lib/{0}/pydaw/python/wavefile/wavefile.pyc
/usr/lib/{0}/pydaw/python/wavefile/wavefile.pyo


%doc

""".format(global_pydaw_version_string, global_version_fedora, 
    global_tarball_url, f_native)

f_spec_file = open(global_spec_file, "w")
f_spec_file.write(f_spec_template)
f_spec_file.close()

os.system('cp "{}" "{}"'.format(global_spec_file, global_specs_dir))

os.chdir(global_specs_dir)
os.system("rpmbuild -ba {}".format(global_spec_file))

#Restore the ~/.rpmmacros file to it's original state.
if global_macro_text is not None:
    global_rpmmacros_file = open("{}/.rpmmacros".format(global_home), "w")
    global_rpmmacros_file.write(global_macro_text)
