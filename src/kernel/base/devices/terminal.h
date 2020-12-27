//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-18-2001                                                      //
// Header:  base/devices/terminal.h                                         //
// Purpose: This is the definition of a terminal device.                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_TERMINAL_H_
#define _BASE_TERMINAL_H_

#include <base/configure.h>
#include <base/util/vmi.h>

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

#define TERMINAL_VTBL_ENTRIES(type) \
  VTBL_ENTRY(type,init), \
  VTBL_ENTRY(type,open), \
  VTBL_ENTRY(type,puts), \
  VTBL_ENTRY(type,putch), \
  VTBL_ENTRY(type,clear), \
  VTBL_ENTRY(type,movecursor), \
  VTBL_ENTRY(type,setattr), \
  VTBL_ENTRY(type,close) 

typedef struct terminal {

  struct terminal_vtbl {
    
    word (*init)      (struct terminal *);
    word (*open)      (struct terminal *);
    word (*puts)      (struct terminal *, cstr);
    word (*putch)     (struct terminal *, char);
    word (*clear)     (struct terminal *);
    word (*movecursor)(struct terminal *, word col, word row);
    word (*setattr)   (struct terminal *, word attr);
    word (*close)     (struct terminal *);

  } *vtbl;

  word type;

} terminal;

#define terminal_init(this) VMI_DISPATCH_O(terminal,init,this)
#define terminal_open(this) VMI_DISPATCH_O(terminal,open,this)
#define terminal_puts(this,a) VMI_DISPATCH_1(terminal,init,this,a)
#define terminal_putch(this,a) VMI_DISPATCH_1(terminal,init,this,a)
#define terminal_clear(this)  VMI_DISPATCH_O(terminal,init,this)
#define terminal_movecursor(this,a,b) VMI_DISPATCH_2(terminal,init,this,a,b)
#define terminal_setattr(this,a) VMI_DISPATCH_1(terminal,init,this,a)
#define terminal_close(this) VMI_DISPATCH_O(terminal,init,this)

#endif
