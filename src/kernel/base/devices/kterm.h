//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-18-2001                                                      //
// Header:  base/devices/kterm.h                                            //
// Purpose: This is the standard kernel terminal for output.                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_KTERM_H_
#define _BASE_KTERM_H_

#include <base/devices/terminal.h>

word kterm_init(void);
word kterm_print(cstr fmt, ...);
word kterm_putch(char c);
word kterm_clear(void);
word kterm_getcursor(word *col, word *row);
word kterm_movecursor(word col, word row);
word kterm_movecol(word col);
word kterm_moverow(word row);
word kterm_scrollup(word lines);
word kterm_scrolldown(word lines);
word kterm_setattr(word attr);

#endif
