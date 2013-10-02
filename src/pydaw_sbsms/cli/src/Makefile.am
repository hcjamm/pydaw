AM_CXXFLAGS = @SBSMS_CFLAGS@ @SBSMS_CFLAGS@ @SNDFILE_CFLAGS@ -I../include

lib_LTLIBRARIES = libsbsmsx.la

libsbsmsx_la_SOURCES = import.cpp import.h pcm.cpp pcm.h convert.cpp convert.h audiobuffer.cpp audiobuffer.h

bin_PROGRAMS = sbsms

sbsms_LDADD = libsbsmsx.la @SBSMS_LIBS@ @SNDFILE_LIBS@
sbsms_LDFLAGS = $(os_ldflags)
sbsms_SOURCES = test.cpp 
