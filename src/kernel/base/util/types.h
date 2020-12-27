//////////////////////////////////////////////////////////////////////////////
// Project: Codename Red Pants                                              //
// Header:  base/types.h                                                    //
// Date:    4-28-1999                                                       //
// Purpose: This header contains the typedefs for the standard variables    //
//          which is included in most of Redpants' utils.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_TYPES_H_
#define _BASE_TYPES_H_

#define _UNIX_BUILD_

#ifndef NULL
 #define NULL 0
 #define null 0
#endif

#define extends : public
#define BIT(x) (((word)1)<<(x))
#define SETBIT(x, b) ((x)|(1<<(b)))

// Some useful KB, MB, and GB constants
#define _1kb     0x400
#define _2kb     0x800
#define _4kb    0x1000
#define _8kb    0x2000
#define _16kb   0x4000
#define _32kb   0x8000
#define _64kb  0x10000
#define _128kb 0x20000
#define _256kb 0x40000
#define _512kb 0x80000
#define _640kb (10*_64kb)

#define _1mb      0x100000
#define _2mb      0x200000
#define _4mb      0x400000
#define _8mb      0x800000
#define _16mb    0x1000000
#define _32mb    0x2000000
#define _64mb    0x4000000
#define _128mb   0x8000000
#define _256mb  0x10000000
#define _512mb  0x20000000

#define _1gb      0x40000000
#define _2gb      0x80000000
#define _4gb     0x100000000
#define _8gb     0x200000000
#define _16gb    0x400000000
#define _32gb    0x800000000
#define _64gb   0x1000000000
#define _128gb  0x2000000000
#define _256gb  0x4000000000
#define _512gb  0x8000000000

#define _1tb   0x10000000000
#define _2tb   0x20000000000
#define _4tb   0x40000000000

// Architecture constants, do not modify
#define ARCH_IA32   0x01
#define ARCH_X86_64 0x02
#define ARCH_IA64   0x03
#define ARCH_ALPHA  0x04
#define ARCH_PPC    0x05
#define ARCH_SPARC  0x06

#define yes 1
#define no  0
#define off 0
#define on  1

#ifndef __cplusplus
 #define true 1
 #define false 0
#endif

#include <target/types.h>

typedef char * cstr;
typedef byte * bptr;
typedef word * wptr;

// macro for compiler independent packing of structs
#define PACKED(x) __attribute__((packed)) x

#endif
