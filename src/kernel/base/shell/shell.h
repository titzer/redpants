//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-12-2000                                                       //
// Header:  base/shell/shell.h                                              //
// Purpose: This is the kernel shell, the builtin command prompt in the     //
//          operating system.                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _SHELL_H_
#define _SHELL_H_

#include <base/configure.h>
#include <base/shell/hashtable.h>

#ifdef _SHELL_ANCHOR_
 #define sglobal
#else
 #define sglobal extern
#endif

sglobal hashtable *commands;
sglobal word shell_buildnum;

#define command_stub_type(x) void (*x)(cstr args)
#define command_stub(x) void x(cstr args)

void start_kernel_shell(void);
void print_shell_banner(void);
void insert_commands(void);
void insert_command(cstr s, void *hook);

#endif
