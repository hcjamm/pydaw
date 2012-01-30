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
plugin_LTLIBRARIES = ray_v.la
else
plugin_LTLIBRARIES = ray_v.la
endif

ray_v_la_SOURCES = \
        synth.c \
	../dssi/dssi.h



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
install-exec-hook:
	if test -x $(DESTDIR)$(bindir)/jack-dssi-host$(EXEEXT) ; then \
	  cd $(DESTDIR)$(bindir) ; \	  
	  if ! test -x ./ray_v$(EXEEXT) ; then \
	    $(LN_S) jack-dssi-host$(EXEEXT) ray_v$(EXEEXT) ; \
	  fi ; \	  
	fi

