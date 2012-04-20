## Process this file with automake to produce Makefile.in

plugindir = $(libdir)/dssi

plugin_LTLIBRARIES = euphoria.la

euphoria_la_SOURCES = \
	synth.c \
	synth.h \
	dssi.h

euphoria_la_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(ALSA_CFLAGS) $(SNDFILE_CFLAGS) $(SRC_CFLAGS)

euphoria_la_LDFLAGS = -module -avoid-version

euphoria_la_LIBADD = -lpthread -lm $(SNDFILE_LIBS) $(SRC_LIBS)

if HAVE_LIBLO
if HAVE_QT
euphoria_ui_PROGRAMS = LMS_EUPHORIA_qt
else
euphoria_ui_PROGRAMS =
endif
else
euphoria_ui_PROGRAMS =
endif

euphoria_uidir = $(libdir)/dssi/euphoria

euphoria_MOC = synth_qt_gui.moc.cpp

LMS_EUPHORIA_qt_SOURCES = \
	synth_qt_gui.cpp \
	synth_qt_gui.h \
	dssi.h

nodist_LMS_EUPHORIA_qt_SOURCES = $(euphoria_MOC)

LMS_EUPHORIA_qt_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/dssi $(ALSA_CFLAGS) $(QT_CFLAGS) $(LIBLO_CFLAGS) $(SNDFILE_CFLAGS)
LMS_EUPHORIA_qt_LDADD = $(AM_LDFLAGS) $(QT_LIBS) $(LIBLO_LIBS) $(SNDFILE_LIBS)

%.moc.cpp: %.h
	$(QT_MOC) -o $@ $<

if HAVE_QT
BUILT_SOURCES = $(euphoria_MOC)
else
BUILT_SOURCES =
endif

CLEANFILES = $(BUILT_SOURCES)
