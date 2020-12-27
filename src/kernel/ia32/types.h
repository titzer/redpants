//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-25-2001                                                      //
// Header:  ia32/types.h                                                    //
// Purpose: This header declares primitive integer types for use in the     //
//          kernel that are of the appropriate size for the Intel arch.     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_TYPES_H_
#define _IA32_TYPES_H_

// These are fixed size, regardless of machine
typedef char               s8i;  // (s)igned   (8)bit  (i)nteger
typedef unsigned char      u8i;  // (u)nsigned (8)bit  (i)nteger
typedef signed   short     s16i; // (s)igned   (16)bit (i)nteger
typedef unsigned short     u16i; // (u)nsigned (16)bit (i)nteger
typedef signed   long      s32i; // (s)igned   (32)bit (i)nteger
typedef unsigned long      u32i; // (u)nsigned (32)bit (i)nteger
typedef signed   long long s64i; // (s)igned   (64)bit (i)nteger 
typedef unsigned long long u64i; // (u)nsigned (64)bit (i)nteger

// These are variable size, machine dependent
typedef unsigned char  byte; // always a byte
typedef unsigned short half; // always half a word
typedef unsigned int   word; // always a machine word (eg, 32 bits on IA-32)

// Definitions for floating point types
typedef float       s32f; // (s)igned (32)bit (f)loating point
typedef double      s64f; // (s)igned (64)bit (f)loating point
typedef long double s80f; // (s)igned (80)bit (f)loating point

typedef float stdflt;


#endif
