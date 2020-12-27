//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-07-2001                                                       //
// Module:  util/kprint.c                                                   //
// Purpose: This utility is for printing strings, data and other things     //
//          through a common mechanism for all of the kernel.               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/util/kprint.h>
#include <base/devices/kterm.h>
#include <stdarg.h>

// kprint format arguments:
//
// %s - string
// %d - integer
// %l - long (64 bit integer)
// %c - character
// %k - change console color
// %f - floating point
// %1x - 8 bit hex integer
// %2x - 16 bit hex integer
// %4x - 32 bit hex integer
// %8x - 64 bit hex integer

byte hexdigits[16] =
 { '0', '1', '2', '3', '4', '5', '6', '7',
   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void sprint(cstr dest, cstr fmt, ...)
{
  //  va_list ap;

}

void kprint_u32i(u32i k)
{
}

void kprint_s32i(s32i k)
{
}

void kprint_u64i(u64i k)
{
}

void kprint_s64i(u64i k)
{
}

void kprint_u8x(u8i k)
{
}

void kprint_u16x(u16i k)
{
}

void kprint_u32x(u32i k)
{
}

void kprint_u64x(u64i k)
{
}

void kprint_s32f(s32f k)
{
}

void kprint_s64f(s64f k)
{
}

void kprint_s80f(s80f k)
{
}

void kprint_char(char k)
{
}

void kprint_str(cstr str)
{
}

void kprint(cstr fmt, ...)
{
  //  va_list ap;
  //  char buffer[64];
  cstr str = fmt;

  while ( *str ) // loop through format string
     {
     if ( *str == '%' ) // expand parameter
        {
        str++;
        if ( *str == '%' ) kterm_putch('%');
        else switch ( *str )
	  {
	  default: break;
	  }
        }
     else // not a parameter, print single character
        {
        kterm_putch(*str);
        str++;
        }
     }
}

