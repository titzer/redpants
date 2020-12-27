//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-02-2000                                                       //
// Header:  base/util/debug.h                                               //
// Purpose: This module contains helper macros and functions that allow     //
//          portions of the kernel to be debugged by altering switches      //
//          in configure.h.                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_DEBUG_H_
#define _BASE_DEBUG_H_

#include <base/util/kprint.h>

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Notes:                                                                  //
//                                                                          //
//    These macros generate calls to debugging functions contained in       //
//  debug.c, based on the switch DEBUG_SWITCH. If DEBUG_NONE is true, then  //
//  no calls to debugging functions are generated (nothing is debugged).    //
//  If DEBUG_ALL is true, all calls to debug macros generate calls to       //
//  debugging functions. The module defines the macro DEBUG_SWITCH, and     //
//  thus has control over whether the debug code is generated.              //
//    Most modules will set this switch as another macro from configure.h   //
//  so that the debugging of all modules can be controlled from configure.h //
//  alone.                                                                  //
//    The module can then use the macros to print out debugging             //
//  information. The module can also use the macro DEBUG as a switch to     //
//  include/exclude debugging code.                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#if ( DEBUG_NONE )

// if DEBUG_NONE don't generate code for these debugging macros.
 #define DEBUG 0
 #define DEBUG_PRINT(x)
 #define DEBUG_STR(x)
 #define DEBUG_INT(x)
 #define DEBUG_H32(x)
 #define DEBUG_H16(x)
 #define DEBUG_H8(x)
 #define DEBUG_DEL(x)
 #define DEBUG_DELAY(x)

#elif ( DEBUG_ALL || DEBUG_SWITCH )

// if debug all, generate code for all debugging macros
 #define DEBUG 1
 #define DEBUG_PRINT(x)\
   printstring("\n" DEBUG_MODULE "_DEBUG: ",DEBUG_COLOR);\
   printstring(x, DEBUG_COLOR)
 #define DEBUG_STR(x)   printstring(x, DEBUG_COLOR)
 #define DEBUG_INT(x)   printlong(x, DEBUG_COLOR)
 #define DEBUG_H32(x)   printhex32(x, DEBUG_COLOR)
 #define DEBUG_H16(x)   printhex16(x, DEBUG_COLOR)
 #define DEBUG_H8(x)    printhex8(x, DEBUG_COLOR)
 #define DEBUG_DEL(x)   pit_delay(x)
 #define DEBUG_DELAY(x) pit_delay(x*DEBUG_STD_DELAY)

#else

 #define DEBUG 0
 #define DEBUG_PRINT(x)
 #define DEBUG_STR(x)
 #define DEBUG_INT(x)
 #define DEBUG_H32(x)
 #define DEBUG_H16(x)
 #define DEBUG_H8(x)
 #define DEBUG_DEL(x)
 #define DEBUG_DELAY(x)

#endif


#endif
