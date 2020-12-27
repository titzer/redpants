//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-13-2000                                                       //
// Header:  ia32/devices/console.h                                          //
// Purpose: This header contains declarations for the portions of the       //
//          kernel that handle interaction with the console display.        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_CONSOLE_H_
#define _IA32_CONSOLE_H_

#include <base/configure.h>
#include <base/util/types.h>

#define TEXT_DEFAULT 0xff

// color constants
#define BLACK        0x00
#define BLUE         0x01
#define GREEN        0x02
#define CYAN         0x03
#define RED          0x04
#define MAGENTA      0x05
#define BROWN        0x06
#define LIGHTGRAY    0x07
#define DARKGRAY     0x08
#define LIGHTBLUE    0x09
#define LIGHTGREEN   0x0A
#define LIGHTCYAN    0x0B
#define LIGHTRED     0x0C
#define LIGHTMAGENTA 0x0D
#define YELLOW       0x0E
#define WHITE        0x0F

// text box used in printing to VGA console
typedef struct _textbox
{
  byte left;
  byte right;
  byte top;
  byte bottom;
  byte attr;
  byte curx;
  byte cury;

} textbox;

// init function
void console_init(void);

// text functions
void console_clear(void);
void console_clear_box(textbox *t);
void console_putch_box(textbox *t, byte c, byte color);
void console_puts_box(textbox *t, cstr str, byte color);
void console_movecursor_box(textbox *t, byte x, byte y);
void console_scrollup_box(textbox *t, byte lines);
void console_scrolldown_box(textbox *t, byte lines);

textbox *console_getfullbox(void);

#endif
