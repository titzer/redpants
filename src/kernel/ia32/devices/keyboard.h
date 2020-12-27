//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-11-2000                                                       //
// Header:  ia32/devices/keyboard.h                                         //
// Purpose: This module of the kernel handles input from the keyboard,      //
//          including the handling of keyboard interrupts and the decoding  //
//          of the scan codes via a scan table. It was based largely on     //
//          the keyboard controller of the GazOS operating system, by       //
//          Gareth Owen. Many thanks to him for his GazOS.                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_KEYBOARD_H_
#define _IA32_KEYBOARD_H_

#include <base/configure.h>

#define KEYBOARD_IRQ 1

// led keyboard indicators
#define LED_SCROLL_LOCK  1
#define LED_NUM_LOCK     2
#define LED_CAPS_LOCK    4

// key modifiers
#define CK_SHIFT  8
#define CK_ALT    16
#define CK_CTRL   32

// key scan codes
#define SCAN_NUM_LOCK    0x45
#define SCAN_CAPS_LOCK   0x3a
#define SCAN_SCROLL_LOCK 0x46
#define SCAN_CTRL        0x1d
#define SCAN_RSHIFT      0x36
#define SCAN_LSHIFT      0x2a
#define SCAN_ALT         0x38
#define SCAN_DEL         0x53

// constants for depressed/released
#define DEPRESSED(a) (a)
#define RELEASED(a) ((a)|0x80)

// pressed, released macros
#define PRESS(a) !((u8i)(a)>>7)
#define RELEASE(a) ((u8i)(a)>>7)

// structure which keyboard handler passes to shell handler
struct keyb_event
{
  byte scancode  __attribute__ ((packed)); // scan code
  byte ctrlstate __attribute__ ((packed)); // state of control keys
  byte asciicode __attribute__ ((packed)); // translated ascii code
  byte extcode   __attribute__ ((packed)); // translated extended (function) code
};

void initialize_keyboard(void); // setup the keyboard

byte getstring(cstr s); // read in a string from keyboard
byte getch(void); // get a character from the keyboard
byte kbhit(void); // check to see if anything is in the buffer

void install_keyhandler(void *); // install shell key handler

#endif
