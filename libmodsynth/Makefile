#This Makefile doesn't actually do anything useful, I only created it so that Netbeans would allow the library to be loaded
#as a project, and provide code completion.  Shared libraries incur processing overhead, LibModSynth is intended to be a
#source-based library with functions that can be inlined into your plugins

#
#  There exist several targets which are by default empty and which can be 
#  used for execution of your targets. These targets are usually executed 
#  before and after some main targets. They are: 
#
#     .build-pre:              called before 'build' target
#     .build-post:             called after 'build' target
#     .clean-pre:              called before 'clean' target
#     .clean-post:             called after 'clean' target
#     .clobber-pre:            called before 'clobber' target
#     .clobber-post:           called after 'clobber' target
#     .all-pre:                called before 'all' target
#     .all-post:               called after 'all' target
#     .help-pre:               called before 'help' target
#     .help-post:              called after 'help' target
#
#  Targets beginning with '.' are not intended to be called on their own.
#
#  Main targets can be executed directly, and they are:
#  
#     build                    build a specific configuration
#     clean                    remove built files from a configuration
#     clobber                  remove all built files
#     all                      build all configurations
#     help                     print help mesage
#  
#  Targets .build-impl, .clean-impl, .clobber-impl, .all-impl, and
#  .help-impl are implemented in nbproject/makefile-impl.mk.
#
#  Available make variables:
#
#     CND_BASEDIR                base directory for relative paths
#     CND_DISTDIR                default top distribution directory (build artifacts)
#     CND_BUILDDIR               default top build directory (object files, ...)
#     CONF                       name of current configuration
#     CND_PLATFORM_${CONF}       platform name (current configuration)
#     CND_ARTIFACT_DIR_${CONF}   directory of build artifact (current configuration)
#     CND_ARTIFACT_NAME_${CONF}  name of build artifact (current configuration)
#     CND_ARTIFACT_PATH_${CONF}  path to build artifact (current configuration)
#     CND_PACKAGE_DIR_${CONF}    directory of package (current configuration)
#     CND_PACKAGE_NAME_${CONF}   name of package (current configuration)
#     CND_PACKAGE_PATH_${CONF}   path to package (current configuration)
#
# NOCDDL


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin


# build
build: .build-post

.build-pre:
# Add your pre 'build' code here...

.build-post: .build-impl
# Add your post 'build' code here...


# clean
clean: .clean-post

.clean-pre:
# Add your pre 'clean' code here...

.clean-post: .clean-impl
# Add your post 'clean' code here...


# clobber
clobber: .clobber-post

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all: .all-post

.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...


# build tests
build-tests: .build-tests-post

.build-tests-pre:
# Add your pre 'build-tests' code here...

.build-tests-post: .build-tests-impl
# Add your post 'build-tests' code here...


# run tests
test: .test-post

.test-pre:
# Add your pre 'test' code here...

.test-post: .test-impl
# Add your post 'test' code here...


# help
help: .help-post

.help-pre:
# Add your pre 'help' code here...

.help-post: .help-impl
# Add your post 'help' code here...



# include project implementation makefile
# 
# Generated Makefile - do not edit! 
# 
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a pre- and a post- target defined where you can add customization code.
#
# This makefile implements macros and targets common to all configurations.
#
# NOCDDL


# Building and Cleaning subprojects are done by default, but can be controlled with the SUB
# macro. If SUB=no, subprojects will not be built or cleaned. The following macro
# statements set BUILD_SUB-CONF and CLEAN_SUB-CONF to .build-reqprojects-conf
# and .clean-reqprojects-conf unless SUB has the value 'no'
SUB_no=NO
SUBPROJECTS=${SUB_${SUB}}
BUILD_SUBPROJECTS_=.build-subprojects
BUILD_SUBPROJECTS_NO=
BUILD_SUBPROJECTS=${BUILD_SUBPROJECTS_${SUBPROJECTS}}
CLEAN_SUBPROJECTS_=.clean-subprojects
CLEAN_SUBPROJECTS_NO=
CLEAN_SUBPROJECTS=${CLEAN_SUBPROJECTS_${SUBPROJECTS}}


# Project Name
PROJECTNAME=CppDynamicLibrary_1

# Active Configuration
DEFAULTCONF=Debug
CONF=${DEFAULTCONF}

# All Configurations
ALLCONFS=Debug Release 


# build
.build-impl: .build-pre .validate-impl .depcheck-impl
	@#echo "=> Running $@... Configuration=$(CONF)"
	"${MAKE}" -f nbproject/Makefile-${CONF}.mk QMAKE=${QMAKE} SUBPROJECTS=${SUBPROJECTS} .build-conf


# clean
.clean-impl: .clean-pre .validate-impl .depcheck-impl
	@#echo "=> Running $@... Configuration=$(CONF)"
	"${MAKE}" -f nbproject/Makefile-${CONF}.mk QMAKE=${QMAKE} SUBPROJECTS=${SUBPROJECTS} .clean-conf


# clobber 
.clobber-impl: .clobber-pre .depcheck-impl
	@#echo "=> Running $@..."
	for CONF in ${ALLCONFS}; \
	do \
	    "${MAKE}" -f nbproject/Makefile-$${CONF}.mk QMAKE=${QMAKE} SUBPROJECTS=${SUBPROJECTS} .clean-conf; \
	done

# all 
.all-impl: .all-pre .depcheck-impl
	@#echo "=> Running $@..."
	for CONF in ${ALLCONFS}; \
	do \
	    "${MAKE}" -f nbproject/Makefile-$${CONF}.mk QMAKE=${QMAKE} SUBPROJECTS=${SUBPROJECTS} .build-conf; \
	done

# build tests
.build-tests-impl: .build-impl .build-tests-pre
	@#echo "=> Running $@... Configuration=$(CONF)"
	"${MAKE}" -f nbproject/Makefile-${CONF}.mk SUBPROJECTS=${SUBPROJECTS} .build-tests-conf

# run tests
.test-impl: .build-tests-impl .test-pre
	@#echo "=> Running $@... Configuration=$(CONF)"
	"${MAKE}" -f nbproject/Makefile-${CONF}.mk SUBPROJECTS=${SUBPROJECTS} .test-conf

# dependency checking support
.depcheck-impl:
	@echo "# This code depends on make tool being used" >.dep.inc
	@if [ -n "${MAKE_VERSION}" ]; then \
	    echo "DEPFILES=\$$(wildcard \$$(addsuffix .d, \$${OBJECTFILES}))" >>.dep.inc; \
	    echo "ifneq (\$${DEPFILES},)" >>.dep.inc; \
	    echo "include \$${DEPFILES}" >>.dep.inc; \
	    echo "endif" >>.dep.inc; \
	else \
	    echo ".KEEP_STATE:" >>.dep.inc; \
	    echo ".KEEP_STATE_FILE:.make.state.\$${CONF}" >>.dep.inc; \
	fi

# configuration validation
.validate-impl:
	@if [ ! -f nbproject/Makefile-${CONF}.mk ]; \
	then \
	    echo ""; \
	    echo "Error: can not find the makefile for configuration '${CONF}' in project ${PROJECTNAME}"; \
	    echo "See 'make help' for details."; \
	    echo "Current directory: " `pwd`; \
	    echo ""; \
	fi
	@if [ ! -f nbproject/Makefile-${CONF}.mk ]; \
	then \
	    exit 1; \
	fi


# help
.help-impl: .help-pre
	@echo "This makefile supports the following configurations:"
	@echo "    ${ALLCONFS}"
	@echo ""
	@echo "and the following targets:"
	@echo "    build  (default target)"
	@echo "    clean"
	@echo "    clobber"
	@echo "    all"
	@echo "    help"
	@echo ""
	@echo "Makefile Usage:"
	@echo "    make [CONF=<CONFIGURATION>] [SUB=no] build"
	@echo "    make [CONF=<CONFIGURATION>] [SUB=no] clean"
	@echo "    make [SUB=no] clobber"
	@echo "    make [SUB=no] all"
	@echo "    make help"
	@echo ""
	@echo "Target 'build' will build a specific configuration and, unless 'SUB=no',"
	@echo "    also build subprojects."
	@echo "Target 'clean' will clean a specific configuration and, unless 'SUB=no',"
	@echo "    also clean subprojects."
	@echo "Target 'clobber' will remove all built files from all configurations and,"
	@echo "    unless 'SUB=no', also from subprojects."
	@echo "Target 'all' will will build all configurations and, unless 'SUB=no',"
	@echo "    also build subprojects."
	@echo "Target 'help' prints this message."
	@echo ""


# include project make variables

#
# Generated - do not edit!
#
# NOCDDL
#
CND_BASEDIR=`pwd`
CND_BUILDDIR=build
CND_DISTDIR=dist
# Debug configuration
CND_PLATFORM_Debug=GNU-Linux-x86
CND_ARTIFACT_DIR_Debug=dist/Debug/GNU-Linux-x86
CND_ARTIFACT_NAME_Debug=libCppDynamicLibrary_1.so
CND_ARTIFACT_PATH_Debug=dist/Debug/GNU-Linux-x86/libCppDynamicLibrary_1.so
CND_PACKAGE_DIR_Debug=dist/Debug/GNU-Linux-x86/package
CND_PACKAGE_NAME_Debug=libCppDynamicLibrary1.so.tar
CND_PACKAGE_PATH_Debug=dist/Debug/GNU-Linux-x86/package/libCppDynamicLibrary1.so.tar
# Release configuration
CND_PLATFORM_Release=GNU-Linux-x86
CND_ARTIFACT_DIR_Release=dist/Release/GNU-Linux-x86
CND_ARTIFACT_NAME_Release=libCppDynamicLibrary_1.so
CND_ARTIFACT_PATH_Release=dist/Release/GNU-Linux-x86/libCppDynamicLibrary_1.so
CND_PACKAGE_DIR_Release=dist/Release/GNU-Linux-x86/package
CND_PACKAGE_NAME_Release=libCppDynamicLibrary1.so.tar
CND_PACKAGE_PATH_Release=dist/Release/GNU-Linux-x86/package/libCppDynamicLibrary1.so.tar
#
# include compiler specific variables
#
# dmake command
ROOT:sh = test -f nbproject/private/Makefile-variables.mk || \
	(mkdir -p nbproject/private && touch nbproject/private/Makefile-variables.mk)
#
# gmake command
.PHONY: $(shell test -f nbproject/private/Makefile-variables.mk || (mkdir -p nbproject/private && touch nbproject/private/Makefile-variables.mk))
#
include nbproject/private/Makefile-variables.mk

