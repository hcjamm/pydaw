dnl Process this file with autoconf to produce a configure script.

AC_INIT(DSSI, 1.1.1, dssi-devel@lists.sourceforge.net)

AC_CONFIG_SRCDIR(dssi/dssi.h)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CXX
AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)
AC_PROG_LIBTOOL
AC_PROG_LN_S

AC_C_BIGENDIAN

dnl Check for LADSPA
AC_CHECK_HEADERS(ladspa.h)

dnl Require ALSA or libdssialsacompat
PKG_CHECK_MODULES(ALSA, alsa >= 0.9, with_alsa=ALSA, with_alsa=no)
if test "x${with_alsa}" = xno ; then
  PKG_CHECK_MODULES(ALSA, libdssialsacompat, with_alsa=libdssialsacompat,
                    AC_MSG_ERROR([neither ALSA nor libdssialsacompat found]))
fi
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)

dnl Check for liblo
PKG_CHECK_MODULES(LIBLO, liblo >= 0.12, with_liblo=yes, with_liblo=no)
AC_SUBST(LIBLO_CFLAGS)
AC_SUBST(LIBLO_LIBS)
AM_CONDITIONAL(HAVE_LIBLO, test x$with_liblo = xyes)

dnl Check for JACK
PKG_CHECK_MODULES(JACK, jack >= 0.99.14, with_jack=yes, with_jack=no)
AC_SUBST(JACK_CFLAGS)
AC_SUBST(JACK_LIBS)
AM_CONDITIONAL(HAVE_JACK, test x$with_jack = xyes)

dnl Check for libsndfile and libsamplerate for trivial_sampler
PKG_CHECK_MODULES(SNDFILE, sndfile, with_sndfile=yes, with_sndfile=no)
PKG_CHECK_MODULES(SRC, samplerate, with_SRC=yes, with_SRC=no)
AC_SUBST(SNDFILE_CFLAGS)
AC_SUBST(SNDFILE_LIBS)
AC_SUBST(SRC_CFLAGS)
AC_SUBST(SRC_LIBS)

dnl Check for Qt
with_qt=no
PKG_CHECK_MODULES(QT, [QtCore >= 4.0.1 QtGui >= 4.0.1],
    [with_qt=yes
     QT_MOC=`$PKG_CONFIG --variable=moc_location QtCore`],
    AC_MSG_NOTICE([pkg-config could not find Qt >= 4.0.1]))
if test "x${with_qt}" = xno ; then
  case "${host_os}" in
  darwin*)
    AC_MSG_NOTICE([checking for Qt OS X frameworks])
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    dssi_save_cppflags="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I/Library/Frameworks/QtCore.framework/Headers"
    CPPFLAGS="$CPPFLAGS -I/Library/Frameworks/QtGui.framework/Headers"
    AC_CHECK_HEADERS(QApplication, [with_qt=yes])
    CPPFLAGS="$dssi_save_cppflags"
    AC_LANG_RESTORE
    if test "x${with_qt}" = xyes ; then
      AC_PATH_PROG(QT_MOC, moc)
      if test "x$QT_MOC" = x ; then
        echo moc not found
        with_qt=no
      else
        QT_CFLAGS="-I/Library/Frameworks/QtCore.framework/Headers"
        QT_CFLAGS="$QT_CFLAGS -I/Library/Frameworks/QtGui.framework/Headers"
        QT_LIBS="-Xlinker -framework -Xlinker QtCore -Xlinker -framework -Xlinker QtGui"
      fi
    fi
    ;;
  esac
fi
if test "x${with_qt}" = xyes ; then
  dnl both less_trivial_synth_qt_gui and trivial_sampler_qt_gui need
  dnl '-lX11' in QT_LIBS when on an X11 system (because they both
  dnl utilize X11 directly; we check if Q_WS_X11 is defined.)
  AC_MSG_CHECKING([whether Qt is X11-based])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  dssi_save_cxxflags="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS $QT_CFLAGS"
  AC_TRY_COMPILE([#include <QApplication>],
      [#ifndef Q_WS_X11
       #error notX11
       #endif
      ], [QT_LIBS="$QT_LIBS -lX11"
          AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])])
  CXXFLAGS="$dssi_save_cxxflags"
  AC_LANG_RESTORE
fi
AC_SUBST(QT_CFLAGS)
AC_SUBST(QT_LIBS)
AC_SUBST(QT_MOC)
AM_CONDITIONAL(HAVE_QT, test x$with_qt = xyes)

dnl OS specific checks
case "${host_os}" in
darwin*)
  darwin=yes
  AC_DEFINE(MIDI_COREMIDI, 1, [Define for CoreMIDI MIDI support])
  ;;
*)
  darwin=no
  if test "x${with_alsa}" = xALSA ; then
    AC_DEFINE(MIDI_ALSA, 1, [Define for ALSA MIDI support])
  fi
  ;;
esac
AM_CONDITIONAL(DARWIN, test x$darwin = xyes)

dnl Set GCC warning flags
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
    *[\ \	]-Wall[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

jack_dssi_host_needs=''
dssi_osc_tools_needs=''
trivial_sampler_needs=''
less_trivial_synth_gui_needs=''
trivial_sampler_gui_needs=''
if test "x$with_liblo" != xyes ; then
    jack_dssi_host_needs="$jack_dssi_host_needs liblo"
    dssi_osc_tools_needs="$dssi_osc_tools_needs liblo"
    less_trivial_synth_gui_needs="$less_trivial_synth_gui_needs liblo"
    trivial_sampler_gui_needs="$trivial_sampler_gui_needs liblo"
fi
if test "x$with_jack" != xyes ; then
    jack_dssi_host_needs="$jack_dssi_host_needs JACK"
fi
if test "x$with_sndfile" != xyes ; then
    trivial_sampler_needs="$trivial_sampler_needs libsndfile"
    trivial_sampler_gui_needs="$trivial_sampler_gui_needs libsndfile"
fi
if test "x$with_SRC" != xyes ; then
    trivial_sampler_needs="$trivial_sampler_needs libsamplerate"
fi
AM_CONDITIONAL(BUILD_SAMPLER, test -z "$trivial_sampler_needs")
if test "x$with_qt" != xyes ; then
    less_trivial_synth_gui_needs="$less_trivial_synth_gui_needs Qt4"
    trivial_sampler_gui_needs="$trivial_sampler_gui_needs Qt4"
fi

AC_OUTPUT([
    Makefile
    dssi.pc
    jack-dssi-host/Makefile
    examples/Makefile
    tests/Makefile
])

dnl --- Below here is just pretty output for the user ---

echo "* ======== DSSI ${PACKAGE_VERSION} configured ========"

if test -z "$jack_dssi_host_needs" ; then
    echo "* building jack-dssi-host:            yes"
else
    echo "* building jack-dssi-host:            no (missing$jack_dssi_host_needs)"
fi
echo "* building DSSI list/analyse tools:   yes"
if test -z "$dssi_osc_tools_needs" ; then
    echo "* building DSSI OSC tools:            yes"
else
    echo "* building DSSI OSC tools:            no (missing$dssi_osc_tools_needs)"
fi
echo "* building trivial_synth plugin:      yes"
echo "* building less_trivial_synth plugin: yes"
echo "* building karplong plugin:           yes"
if test -z "$trivial_sampler_needs" ; then
    echo "* building trivial_sampler plugin:    yes"
else
    echo "* building trivial_sampler plugin:    no (missing$trivial_sampler_needs)"
fi
if test -z "$less_trivial_synth_gui_needs" ; then
    echo "* building less_trivial_synth GUI:    yes"
else
    echo "* building less_trivial_synth GUI:    no (missing$less_trivial_synth_gui_needs)"
fi
if test -z "$trivial_sampler_gui_needs" ; then
    echo "* building trivial_sampler GUI:       yes"
else
    echo "* building trivial_sampler GUI:       no (missing$trivial_sampler_gui_needs)"
fi
