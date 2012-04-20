
## Process this file with automake to produce Makefile.in

plugindir = $(libdir)/dssi

if BUILD_SAMPLER
plugin_LTLIBRARIES = lms_distortion.la
else
plugin_LTLIBRARIES = lms_distortion.la
endif

lms_distortion_la_SOURCES = \
        synth.c \
	dssi.h

lms_distortion_la_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(ALSA_CFLAGS)

lms_distortion_la_LDFLAGS = -module -avoid-version
if DARWIN
lms_distortion_la_LIBADD = -lm -lmx
else
lms_distortion_la_LIBADD = -lm
endif

if HAVE_LIBLO
if HAVE_QT
lms_ui_PROGRAMS = LMS_DISTORTION_qt
else
lms_ui_PROGRAMS =
endif
else
lms_ui_PROGRAMS =
endif

lms_uidir = $(libdir)/dssi/lms_distortion

LMS_MOC = synth_qt_gui.moc.cpp

LMS_DISTORTION_qt_SOURCES = \
	synth_qt_gui.cpp \
	synth_qt_gui.h

nodist_LMS_DISTORTION_qt_SOURCES = $(LMS_MOC)

LMS_DISTORTION_qt_CXXFLAGS = $(AM_CXXFLAGS) $(QT_CFLAGS) $(LIBLO_CFLAGS)
LMS_DISTORTION_qt_LDADD = $(AM_LDFLAGS) $(QT_LIBS) $(LIBLO_LIBS)

CLEANFILES = $(BUILT_SOURCES)

# create symlinks for each plugin to jack-dssi-host
#install-exec-hook:

