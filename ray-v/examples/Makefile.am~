## Process this file with automake to produce Makefile.in

if HAVE_LIBLO
bin_PROGRAMS = dssi_analyse_plugin dssi_list_plugins dssi_osc_send dssi_osc_update
else
bin_PROGRAMS = dssi_analyse_plugin dssi_list_plugins
endif

dssi_analyse_plugin_SOURCES = dssi_analyse_plugin.c
dssi_analyse_plugin_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(ALSA_CFLAGS)
dssi_analyse_plugin_LDADD = $(AM_LDFLAGS) -ldl

dssi_list_plugins_SOURCES = dssi_list_plugins.c
dssi_list_plugins_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(ALSA_CFLAGS)
dssi_list_plugins_LDADD = $(AM_LDFLAGS) -ldl

dssi_osc_send_SOURCES = dssi_osc_send.c
dssi_osc_send_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(LIBLO_CFLAGS)
dssi_osc_send_LDADD = $(AM_LDFLAGS) $(LIBLO_LIBS)

dssi_osc_update_SOURCES = dssi_osc_update.c
dssi_osc_update_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(LIBLO_CFLAGS)
dssi_osc_update_LDADD = $(AM_LDFLAGS) $(LIBLO_LIBS)

plugindir = $(libdir)/dssi

if BUILD_SAMPLER
plugin_LTLIBRARIES = synth.la
else
plugin_LTLIBRARIES = synth.la
endif

synth_la_SOURCES = \
        synth.c \
	saw.h \
	../dssi/dssi.h



synth_la_CFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(ALSA_CFLAGS)

synth_la_LDFLAGS = -module -avoid-version
if DARWIN
synth_la_LIBADD = -lm -lmx
else
synth_la_LIBADD = -lm
endif


if HAVE_LIBLO
if HAVE_QT
lts_ui_PROGRAMS = LTS_qt
else
lts_ui_PROGRAMS =
endif
else
lts_ui_PROGRAMS =
endif

lts_uidir = $(libdir)/dssi/synth

LTS_MOC = synth_qt_gui.moc.cpp

LTS_qt_SOURCES = \
	synth_qt_gui.cpp \
	synth_qt_gui.h

nodist_LTS_qt_SOURCES = $(LTS_MOC)

LTS_qt_CXXFLAGS = $(AM_CXXFLAGS) $(QT_CFLAGS) $(LIBLO_CFLAGS)
LTS_qt_LDADD = $(AM_LDFLAGS) $(QT_LIBS) $(LIBLO_LIBS)



CLEANFILES = $(BUILT_SOURCES)

# create symlinks for each plugin to jack-dssi-host
install-exec-hook:
	if test -x $(DESTDIR)$(bindir)/jack-dssi-host$(EXEEXT) ; then \
	  cd $(DESTDIR)$(bindir) ; \	  
	  if ! test -x ./synth$(EXEEXT) ; then \
	    $(LN_S) jack-dssi-host$(EXEEXT) synth$(EXEEXT) ; \
	  fi ; \	  
	fi

