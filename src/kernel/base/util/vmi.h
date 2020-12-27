//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    1-03-2002                                                       //
// Header:  base/util/vmi.h                                                 //
// Purpose: This module contains declarations of macros that help make      //
//          dealing with virtual method invocation in C cleaner and easier. //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_VMI_H_
#define _BASE_VMI_H_

#define VMI_DISPATCH_0(type,method,this) \
  this->vtbl->method((type*)this)
#define VMI_DISPATCH_1(type,method,this,a) \
  this->vtbl->method((type*)this,a)
#define VMI_DISPATCH_2(type,method,this,a,b) \
  this->vtbl->method((type*)this,a,b)
#define VMI_DISPATCH_3(type,method,this,a,b,c) \
  this->vtbl->method((type*)this,a,b,c)
#define VMI_DISPATCH_4(type,method,this,a,b,c,d) \
  this->vtbl->method((type*)this,a,b,c,d)
#define VMI_DISPATCH_5(type,method,this,a,b,c,d,e) \
  this->vtbl->method((type*)this,a,b,c,d,e)
#define VMI_DISPATCH_6(type,method,this,a,b,c,d,e,f) \
  this->vtbl->method((type*)this,a,b,c,d,e,f)
#define VMI_DISPATCH_7(type,method,this,a,b,c,d,e,f,g) \
  this->vtbl->method((type*)this,a,b,c,d,e,f,g)
#define VMI_DISPATCH_8(type,method,this,a,b,c,d,e,f,g,h) \
  this->vtbl->method((type*)this,a,b,c,d,e,f,g,h)

#define VTBL_ENTRY(type,method) type##_##method

#endif
