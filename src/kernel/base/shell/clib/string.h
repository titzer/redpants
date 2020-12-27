//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-12-2000                                                       //
// Header:  base/shell/clib/string.h                                        //
// Purpose: This module contains the definitions of the basic C string      //
//          functions that are used throughout the kernel.                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _CLIB_STRING_H_
#define _CLIB_STRING_H_

#include <base/configure.h>

int   strcmp(const char * a, const char * b);
u32i  strlen(const char * a);
char *strcpy(char * a, const char * b);

#endif
