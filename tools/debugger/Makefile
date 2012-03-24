## -*- Makefile -*-
##
## User: bob
## Time: Feb 8, 2012 9:29:58 PM
## Makefile created by Oracle Solaris Studio.
##
## This file is generated automatically.
##


#### Compiler and tool definitions shared by all build targets #####
CC = gcc
BASICOPTS = -g
CFLAGS = $(BASICOPTS)


# Define the target directories.
TARGETDIR_main.c=GNU-amd64-Linux


all: $(TARGETDIR_main.c)/main.c

## Target: main.c
OBJS_main.c =  \
	$(TARGETDIR_main.c)/main.o
SYSLIBS_main.c = -lm 
USERLIBS_main.c = $(SYSLIBS_main.c) 
DEPLIBS_main.c =  
LDLIBS_main.c = $(USERLIBS_main.c)


# Link or archive
$(TARGETDIR_main.c)/main.c: $(TARGETDIR_main.c) $(OBJS_main.c) $(DEPLIBS_main.c)
	$(LINK.c) $(CFLAGS_main.c) $(CPPFLAGS_main.c) -o $@ $(OBJS_main.c) $(LDLIBS_main.c)


# Compile source files into .o files
$(TARGETDIR_main.c)/main.o: $(TARGETDIR_main.c) main.c
	$(COMPILE.c) $(CFLAGS_main.c) $(CPPFLAGS_main.c) -o $@ main.c



#### Clean target deletes all generated files ####
clean:
	rm -f \
		$(TARGETDIR_main.c)/main.c \
		$(TARGETDIR_main.c)/main.o
	rm -f -r $(TARGETDIR_main.c)


# Create the target directory (if needed)
$(TARGETDIR_main.c):
	mkdir -p $(TARGETDIR_main.c)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-amd64-Linux

