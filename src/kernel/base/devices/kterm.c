//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-20-2001                                                      //
// Module:  base/devices/kterm.c                                            //
// Purpose: This is the standard kernel terminal for output.                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/configure.h>
#include <base/devices/kterm.h>

word kterm_init(void)
{
  return INIT_OK;
}

word kterm_print(cstr fmt, ...)
{
  return 0;
}
