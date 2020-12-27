##############################################################################
## Project: Redpants Kernel                                                 ##
## Date:    12-20-2001                                                      ##
## Module:  ia32/makefile                                                   ##
## Purpose: This is the makefile for the IA-32 portion of the kernel.       ##
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
##     RPK_OPTIONS: Options to pass to the C and C++ compilers during       ##
##              compilation of each module.                                 ##
##                                                                          ##
##############################################################################

# Programs to delete and copy files with
REMOVE = rm -f
COPY = cp -f
STRIP = strip

# The object files belonging to the kernel
RPK_CPU = cpu/cpudetect.o cpu/smpdetect.o cpu/cpu.o cpu/ivm.o cpu/traps.o \
	  cpu/cpuid.o cpu/rdtsc.o
RPK_MEMORY = 
RPK_DEVICES = devices/pit.o devices/pic.o devices/rtc.o devices/vgaterm.o

OBJS = $(RPK_CPU) $(RPK_MEMORY) $(RPK_DEVICES)

# The kernel filename
KERNELFN = rpkernel.elf
BASEOBJ  = ../base/rpkbase.o

# Link the kernel statically with fixed text+data address @2M
$(KERNELFN) : $(OBJS) $(BASEOBJ)
	$(RPK_LINK) -o $(KERNELFN) $(OBJS) $(BASEOBJ) -Ttext 0x200000
	$(STRIP) $(KERNELFN)

# Compile Source Files
.cpp.o:
	$(RPK_CPP) $(RPK_OPTIONS) -I.. -c -o $@ $<

.c.o:
	$(RPK_CC) $(RPK_OPTIONS) -I.. -c -o $@ $<

# Clean up the junk
clean:
	$(REMOVE) $(OBJS)

