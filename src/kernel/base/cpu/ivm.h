//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-14-2001                                                       //
// Header:  base/cpu/ivm.h                                                  //
// Purpose: This module of the kernel handles initializing and controlling  //
//          all of the interrupt handling tables and vectors.               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_IVM_H_
#define _BASE_IVM_H_

#include <base/configure.h>

void  ivm_init(void);
void  ivm_setvector(word intnum, void *handler, word flags);
void *ivm_getvector(word intnum);
void  ivm_setirq(byte num, void *handler, word flags);
void *ivm_getirq(byte num);
void  ivm_maskirq(byte num);
void  ivm_unmaskirq(byte num);
void  ivm_sendeoi(byte num);

#endif
