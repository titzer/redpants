//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-06-2001                                                       //
// Header:  base/util/kprint.h                                              //
// Purpose: This utility is for printing strings, data and other things     //
//          through a common mechanism for all of the kernel.               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_KPRINT_H_
#define _BASE_KPRINT_H_

#include <base/util/types.h>

void sprint(cstr dest, cstr fmt, ...);
void kprint(cstr fmt, ...);

extern byte hexdigits[16];

#endif
