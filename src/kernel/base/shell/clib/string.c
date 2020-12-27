//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-12-2000                                                       //
// Module:  string.c                                                        //
// Purpose: This module contains the definitions of the basic C string      //
//          functions that are used throughout the kernel.                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include "string.h"

//////////////////////////////////////////////////////////////////////////////
//  compare two strings: returns 0 if equal                                 //
//////////////////////////////////////////////////////////////////////////////
int strcmp(const char * a, const char * b)
{
  word cntr;
  int diff = 0;
 
  for ( cntr = 0; ; cntr++ )
    {
    diff = a[cntr] - b[cntr];
    if ( (a[cntr] == 0) || (b[cntr] == 0) ) return diff;
    if ( diff ) break;
    }

  return diff;
}

//////////////////////////////////////////////////////////////////////////////
//  compare two strings: returns 0 if equal                                 //
//////////////////////////////////////////////////////////////////////////////
word strlen(const char * a)
{
  u32i cntr = 0;

  while ( a[cntr] ) cntr++;

  return cntr;
}
