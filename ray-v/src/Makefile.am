## Process this file with automake to produce Makefile.in

plugindir = $(libdir)/dssi

if BUILD_SAMPLER
plugin_LTLIBRARIES = ray_v.la
else
plugin_LTLIBRARIES = ray_v.la
endif

ray_v_la_SOURCES = \
        synth.c \
	dssi.h



ray_v_la_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(ALSA_CFLAGS)

ray_v_la_LDFLAGS = -module -avoid-version
if DARWIN
ray_v_la_LIBADD = -lm -lmx
else
ray_v_la_LIBADD = -lm
endif


if HAVE_LIBLO
if HAVE_QT
lms_ui_PROGRAMS = LMS_qt
else
lms_ui_PROGRAMS =
endif
else
lms_ui_PROGRAMS =
endif

lms_uidir = $(libdir)/dssi/ray_v

LMS_MOC = synth_qt_gui.moc.cpp

LMS_qt_SOURCES = \
	synth_qt_gui.cpp \
	synth_qt_gui.h

nodist_LMS_qt_SOURCES = $(LMS_MOC)

LMS_qt_CXXFLAGS = $(AM_CXXFLAGS) $(QT_CFLAGS) $(LIBLO_CFLAGS)
LMS_qt_LDADD = $(AM_LDFLAGS) $(QT_LIBS) $(LIBLO_LIBS)



CLEANFILES = $(BUILT_SOURCES)

# create symlinks for each plugin to jack-dssi-host
#install-exec-hook:

