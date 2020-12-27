##############################################################################
## Project: Redpants Kernel                                                 ##
## Date:    12-20-2001                                                      ##
## Module:  base/makefile                                                   ##
## Purpose: This is the makefile for the machine independent portion of the ##
##          kernel. Note that this makefile expects environment variables   ##
##          to be set with the name of the compiler and options to be       ##
##          passed to the compiler.                                         ##
##                                                                          ##
##############################################################################
##                   Copyright 1999-2002, Ben L. Titzer                     ##
##############################################################################

##############################################################################
##                                                                          ##
##  ENVIRONMENT VARIABLES EXPECTED:                                         ##
##                                                                          ##
##     RPK_CPP: C++ Compiler command for the target architecture.           ##
##                                                                          ##
##     RPK_CC:  C Compiler command for the target architecture.             ##
##                                                                          ##
##     RPK_LINK: Linker to link object files together into a single .o.     ##
##                                                                          ##
##     RPK_OPTIONS: Options to pass to the C and C++ compilers during       ##
##              compilation of each module.                                 ##
##                                                                          ##
##############################################################################


# Programs to delete and copy files with
REMOVE = rm -f
COPY = cp -f

# The object files belonging to the kernel
RPK_MAIN = main.o system.o

RPK_CPU = cpu/task.o
RPK_SHELL = shell/shell.o shell/hashtable.o shell/cmdstubs.o shell/clib/string.o
RPK_MEMORY = memory/memory.o memory/kmm.o
RPK_UTIL = util/kprint.o
RPK_DEVICES = devices/kterm.o

RPK_OBJS = $(RPK_MAIN) $(RPK_DEVICES) $(RPK_CPU) $(RPK_MEMORY)\
	   $(RPK_UTIL) $(RPK_SHELL)

RPK_BASE=rpkbase.o

# Link all the object files together
$(RPK_BASE) : $(RPK_OBJS)
	$(RPK_LINK) -r -o $(RPK_BASE) $(RPK_OBJS)

# Compile Source Files
.cpp.o:
	$(RPK_CPP) $(RPK_OPTIONS) -I.. -c -o $@ $<

.c.o:
	$(RPK_CC) $(RPK_OPTIONS) -I.. -c -o $@ $<

main.o: configure.h main.c kernel.h
	$(RPK_CC) $(RPK_OPTIONS) -c -I..\
	-DBUILD_STR="\"`version/nextversion`\"" \
	main.c

# Clean up the junk
clean:
	$(REMOVE) $(RPK_OBJS) $(RPK_BASE)

