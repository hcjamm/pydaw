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

os.system("./src.sh")

global_pydaw_version_string = "pydaw4"
global_pydaw_version_num = open("src/{}-version.txt".format(
    global_pydaw_version_string)).read().strip()
global_version_fedora = global_pydaw_version_num.replace("-", ".")
global_pydaw_package_name = "{}-{}".format(global_pydaw_version_string, global_version_fedora)

global_home = os.path.expanduser("~")
global_specs_dir = "{}/rpmbuild/SPECS/".format(global_home)
global_sources_dir = "{}/rpmbuild/SOURCES/".format(global_home)

global_tarball_name = "{}-source-code.tar.gz" .format(global_pydaw_package_name,)
global_tarball_url = "http://sourceforge.net/projects/libmodsynth/files/pydaw4/linux/{}".format(
    global_tarball_name)

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


f_spec_template = \
"""
Name:           {}
Version:        {}

Release:        1%{?dist}
Summary:        A digital audio workstation with a full suite of instrument and effects plugins.

License:        GPLv3
URL:            http://sourceforge.net/projects/libmodsynth/
Source0:        {}

Requires:      python3-PyQt4 gcc alsa-lib-devel liblo-devel \
libsndfile-devel gcc-c++ git python3-numpy python3-scipy \
fftw-devel portmidi-devel libsamplerate-devel python3-devel

%description
PyDAW is a full featured audio and MIDI sequencer with a suite of high quality
instrument and effects plugins.

%prep
%setup -q

%build
make

%install
export DONT_STRIP=1
rm -rf $RPM_BUILD_ROOT
%make_install

%post
%preun

%files

%defattr(644, root, root)

%attr(4755, root, root) /usr/bin/pydaw4-engine

%attr(755, root, root) /usr/bin/pydaw4
%attr(755, root, root) /usr/bin/pydaw4-engine-dbg
%attr(755, root, root) /usr/bin/pydaw4-engine-no-hw
%attr(755, root, root) /usr/bin/pydaw4-engine-no-root
%attr(755, root, root) /usr/bin/pydaw4-project-recover
%attr(755, root, root) /usr/lib/pydaw4/mixxx/mixxx-launcher.py
%attr(755, root, root) /usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_paulstretch.py
%attr(755, root, root) /usr/lib/pydaw4/pydaw/python/pydaw.py
%attr(755, root, root) /usr/lib/pydaw4/rubberband/bin/rubberband
%attr(755, root, root) /usr/lib/pydaw4/sbsms/bin/sbsms

/usr/lib/pydaw4/mixxx/mixxx.desktop
/usr/lib/pydaw4/presets/MODULEX.pypresets
/usr/lib/pydaw4/presets/RAYV.pypresets
/usr/lib/pydaw4/presets/WAYV.pypresets
/usr/lib/pydaw4/pydaw/python/libpydaw/__init__.py
/usr/lib/pydaw4/pydaw/python/libpydaw/liblo.cpython-33m.so
/usr/lib/pydaw4/pydaw/python/libpydaw/libportaudio.so
/usr/lib/pydaw4/pydaw/python/libpydaw/midicomp
/usr/lib/pydaw4/pydaw/python/libpydaw/portaudio.py
/usr/lib/pydaw4/pydaw/python/libpydaw/portmidi.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_device_dialog.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_gradients.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_history.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_osc.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_ports.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_project.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_util.py
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_widgets.py
/usr/lib/pydaw4/pydaw/python/libpydaw/super_formant_maker.py
/usr/lib/pydaw4/pydaw4-version.txt
/usr/lib/pydaw4/rubberband/include/rubberband/RubberBandStretcher.h
/usr/lib/pydaw4/rubberband/include/rubberband/rubberband-c.h
/usr/lib/pydaw4/rubberband/lib/librubberband.a
/usr/lib/pydaw4/rubberband/lib/librubberband.so
/usr/lib/pydaw4/rubberband/lib/librubberband.so.2
/usr/lib/pydaw4/rubberband/lib/librubberband.so.2.1.0
/usr/lib/pydaw4/rubberband/lib/pkgconfig/rubberband.pc
/usr/lib/pydaw4/themes/default/drop-down.png
/usr/lib/pydaw4/themes/default/euphoria.png
/usr/lib/pydaw4/themes/default/h-fader.png
/usr/lib/pydaw4/themes/default/mute-off.png
/usr/lib/pydaw4/themes/default/mute-on.png
/usr/lib/pydaw4/themes/default/play-off.png
/usr/lib/pydaw4/themes/default/play-on.png
/usr/lib/pydaw4/themes/default/pydaw-knob.png
/usr/lib/pydaw4/themes/default/rayv.png
/usr/lib/pydaw4/themes/default/rec-off.png
/usr/lib/pydaw4/themes/default/rec-on.png
/usr/lib/pydaw4/themes/default/record-off.png
/usr/lib/pydaw4/themes/default/record-on.png
/usr/lib/pydaw4/themes/default/solo-off.png
/usr/lib/pydaw4/themes/default/solo-on.png
/usr/lib/pydaw4/themes/default/spinbox-down.png
/usr/lib/pydaw4/themes/default/spinbox-up.png
/usr/lib/pydaw4/themes/default/stop-off.png
/usr/lib/pydaw4/themes/default/stop-on.png
/usr/lib/pydaw4/themes/default/style.txt
/usr/lib/pydaw4/themes/default/v-fader.png
/usr/lib/pydaw4/themes/default/wav-slider-left.png
/usr/lib/pydaw4/themes/default/wav-slider-right.png
/usr/share/applications/pydaw4.desktop
/usr/share/doc/pydaw4/copyright
/usr/share/doc/pydaw4/readme.txt
/usr/share/pixmaps/pydaw4.png

#These seem like undesirable files to package, but it seems that rpmbuild
#creates them even when I go to great lengths to ensure they're not in the
#source tarball?  I guess that makes sense from a load-time perspective
#to have the bytecode cache files, since only root would be able to create them
#once installed.

/usr/lib/pydaw4/mixxx/mixxx-launcher.pyc
/usr/lib/pydaw4/mixxx/mixxx-launcher.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/__init__.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/__init__.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/portaudio.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/portaudio.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/portmidi.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/portmidi.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_device_dialog.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_device_dialog.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_gradients.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_gradients.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_history.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_history.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_osc.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_osc.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_paulstretch.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_paulstretch.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_ports.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_ports.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_project.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_project.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_util.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_util.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_widgets.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/pydaw_widgets.pyo
/usr/lib/pydaw4/pydaw/python/libpydaw/super_formant_maker.pyc
/usr/lib/pydaw4/pydaw/python/libpydaw/super_formant_maker.pyo
/usr/lib/pydaw4/pydaw/python/pydaw.pyc
/usr/lib/pydaw4/pydaw/python/pydaw.pyo

%doc

""".format(global_pydaw_version_string, global_version_fedora, global_tarball_url)

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
