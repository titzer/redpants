//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-11-2000                                                       //
// Module:  keyboard.c                                                      //
// Purpose: This module of the kernel handles input from the keyboard,      //
//          including the handling of keyboard interrupts and the decoding  //
//          of the scan codes via a scan table. It was originally based on  //
//          the keyboard controller of the GazOS operating system by        //
//          Gareth Owen. Many thanks to him for his GazOS.                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Notes: The keyboard interrupt handler defined in this module has the     //
//        responsibility of decoding the scan code into an ascii value,     //
//        setting the appropriate status switches, and then calling an      //
//        event handler which handles keyboard events.                      //
//        This event handler is installed by the shell and the shell uses   //
//        this handler to keep track of keyboard buffers for various        //
//        applications and processes that may be running.                   //
//        The kernel does not keep a record of key presses of any kind,     //
//        its purpose is solely to translate the keypresses to a more       //
//        useful format for the installed handler to work with.             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <devices/keyboard.h>
#include <devices/pic.h>
#include <devices/io.h>
#include <kernel.h>
#include <system.h>
#include <cpu/ivm.h>

#define SCAN_BUFSIZE 12

// local data
byte LEDstatus = 0;
volatile byte keybuffer[KB_BUFFER_SIZE];
volatile byte scanbuffer[SCAN_BUFSIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile byte kb_bufsize = 0;
volatile byte controlkeys;

// handler function for key events
void (*keyevent_handler)(struct keyb_event);

// local functions
INTERRUPT_HANDLER(keyboard_handler);

void update_leds(void);
void print_controlstatus(byte key);
void queuekey(byte key);
void printhex(byte val, byte attr);
u16i getasciicode(byte key); 

// scan code table
u16i scan_table[][8] =
{
/* ASCII, Shift, Ctrl, Alt, Num,	Caps,	Shift Caps,	Shift Num */
{ 0x00, 0x00, 0x00,   0x00,   0x00, 0x00, 0x00, 0x00 },
{ 0x1B, 0x1B, 0x1B,   0x00,   0x1B, 0x1B, 0x1B, 0x1B },
/* 1 -> 9 */
{ 0x31, 0x21, 0x0000, 0x7800, 0x31, 0x31, 0x21, 0x21 },
{ 0x32, 0x40, 0x0300, 0x7900, 0x32, 0x32, 0x40, 0x40 },
{ 0x33, 0x23, 0x0000, 0x7A00, 0x33, 0x33, 0x23, 0x23 },
{ 0x34, 0x24, 0x0000, 0x7B00, 0x34, 0x34, 0x24, 0x24 },
{ 0x35, 0x25, 0x0000, 0x7C00, 0x35, 0x35, 0x25, 0x25 },
{ 0x36, 0x5E, 0x001E, 0x7D00, 0x36, 0x36, 0x5E, 0x5E },
{ 0x37, 0x26, 0x0000, 0x7E00, 0x37, 0x37, 0x26, 0x26 },
{ 0x38, 0x2A, 0x0000, 0x7F00, 0x38, 0x38, 0x2A, 0x2A },
{ 0x39, 0x28, 0x0000, 0x8000, 0x39, 0x39, 0x28, 0x28 },
{ 0x30, 0x29, 0x0000, 0x8100, 0x30, 0x30, 0x29, 0x29 },
/* -, =, Bksp, Tab */
{	0x2D,	0x5F,	0x1F,	0x8200,	0x2D,	0x2D,	0x5F,		0x5F},
{	0x3D,	0x2B,	0,	0x8300,	0x3D,	0x3D,	0x2B,		0x2B},
{	0x08,	0x08,	0x7F,	0,	0x08,	0x08,	0x08,		0x08},
{	0x09,	0x0F00,	0,	0,	0x09,	0x09,	0x0F00,		0x0F00},
/*	QWERTYUIOP[] */
{	0x71,	0x51,	0x11,	0x1000,	0x71,	0x51,	0x71,		0x51},
{	0x77,	0x57,	0x17,	0x1100,	0x77,	0x57,	0x77,		0x57},
{	0x65,	0x45,	0x05,	0x1200,	0x65,	0x45,	0x65,		0x45},
{	0x72,	0x52,	0x12,	0x1300,	0x72,	0x52,	0x72,		0x52},
{	0x74,	0x54,	0x14,	0x1400,	0x74,	0x54,	0x74,		0x54},
{	0x79,	0x59,	0x19,	0x1500,	0x79,	0x59,	0x79,		0x59},
{	0x75,	0x55,	0x15,	0x1600,	0x75,	0x55,	0x75,		0x55},
{	0x69,	0x49,	0x09,	0x1700,	0x69,	0x49,	0x69,		0x49},
{	0x6F,	0x4F,	0x0F,	0x1800,	0x6F,	0x4F,	0x6F,		0x4F},
{	0x70,	0x50,	0x10,	0x1900,	0x70,	0x50,	0x70,		0x50},
{	0x5B,	0x7B,	0x1B,	0x0,	0x5B,	0x5B,	0x7B,		0x7B},
{	0x5D,	0x7D,	0x1D,	0,	0x5D,	0x5D,	0x7D,		0x7D},
/* ENTER, CTRL */
{	0x0D,	0x0D,	0x0A,	0,	0x0D,	0x0D,	0x0A,		0x0A},
{	0,	0,	0,	0,	0,	0,	0,		0},
/* ASDFGHJKL;'~ */
{	0x61,	0x41,	0x01,	0x1E00,	0x61,	0x41,	0x61,		0x41},
{	0x73,	0x53,	0x13,	0x1F00,	0x73,	0x53,	0x73,		0x53},
{	0x64,	0x44,	0x04,	0x2000,	0x64,	0x44,	0x64,		0x44},
{	0x66,	0x46,	0x06,	0x2100,	0x66,	0x46,	0x66,		0x46},
{	0x67,	0x47,	0x07,	0x2200,	0x67,	0x47,	0x67,		0x47},
{	0x68,	0x48,	0x08,	0x2300,	0x68,	0x48,	0x68,		0x48},
{	0x6A,	0x4A,	0x0A,	0x2400,	0x6A,	0x4A,	0x6A,		0x4A},
{	0x6B,	0x4B,	0x0B,	0x3500,	0x6B,	0x4B,	0x6B,		0x4B},
{	0x6C,	0x4C,	0x0C,	0x2600,	0x6C,	0x4C,	0x6C,		0x4C},
{	0x3B,	0x3A,	0,	0,	0x3B,	0x3B,	0x3A,		0x3A},
{	0x27,	0x22,	0,	0,	0x27,	0x27,	0x22,		0x22},
{	0x60,	0x7E,	0,	0,	0x60,	0x60,	0x7E,		0x7E},
/* Left Shift*/
{	0x2A,	0,	0,	0,	0,	0,	0,		0},
/* \ZXCVBNM,./ */
{	0x5C,	0x7C,	0x1C,	0,	0x5C,	0x5C,	0x7C,		0x7C},
{	0x7A,	0x5A,	0x1A,	0x2C00,	0x7A,	0x5A,	0x7A,		0x5A},
{	0x78,	0x58,	0x18,	0x2D00,	0x78,	0x58,	0x78,		0x58},
{	0x63,	0x43,	0x03,	0x2E00,	0x63,	0x43,	0x63,		0x43},
{	0x76,	0x56,	0x16,	0x2F00,	0x76,	0x56,	0x76,		0x56},
{	0x62,	0x42,	0x02,	0x3000,	0x62,	0x42,	0x62,		0x42},
{	0x6E,	0x4E,	0x0E,	0x3100,	0x6E,	0x4E,	0x6E,		0x4E},
{	0x6D,	0x4D,	0x0D,	0x3200,	0x6D,	0x4D,	0x6D,		0x4D},
{	0x2C,	0x3C,	0,	0,	0x2C,	0x2C,	0x3C,		0x3C},
{	0x2E,	0x3E,	0,	0,	0x2E,	0x2E,	0x3E,		0x3E},
{	0x2F,	0x3F,	0,	0,	0x2F,	0x2F,	0x3F,		0x3F},
/* Right Shift */
{	0,	0,	0,	0,	0,	0,	0,		0},
/* Print Screen */
{	0,	0,	0,	0,	0,	0,	0,		0},
/* Alt  */
{	0,	0,	0,	0,	0,	0,	0,		0},
/* Space */
{	0x20,	0x20,	0x20,	0,	0x20,	0x20,	0x20,		0x20},
/* Caps */
{	0,	0,	0,	0,	0,	0,	0,		0},
/* F1-F10 */
{	0x3B00,	0x5400,	0x5E00,	0x6800,	0x3B00,	0x3B00,	0x5400,		0x5400},
{	0x3C00,	0x5500,	0x5F00,	0x6900,	0x3C00,	0x3C00,	0x5500,		0x5500},
{	0x3D00,	0x5600,	0x6000,	0x6A00,	0x3D00,	0x3D00,	0x5600,		0x5600},
{	0x3E00,	0x5700,	0x6100,	0x6B00,	0x3E00,	0x3E00,	0x5700,		0x5700},
{	0x3F00,	0x5800,	0x6200,	0x6C00,	0x3F00,	0x3F00,	0x5800,		0x5800},
{	0x4000,	0x5900,	0x6300,	0x6D00,	0x4000,	0x4000,	0x5900,		0x5900},
{	0x4100,	0x5A00,	0x6400,	0x6E00,	0x4100,	0x4100,	0x5A00,		0x5A00},
{	0x4200,	0x5B00,	0x6500,	0x6F00,	0x4200,	0x4200,	0x5B00,		0x5B00},
{	0x4300,	0x5C00,	0x6600,	0x7000,	0x4300,	0x4300,	0x5C00,		0x5C00},
{	0x4400,	0x5D00,	0x6700,	0x7100,	0x4400,	0x4400,	0x5D00,		0x5D00},
/* Num Lock, Scrl Lock */
{	0,	0,	0,	0,	0,	0,	0,		0},
{	0,	0,	0,	0,	0,	0,	0,		0},
/* HOME, Up, Pgup, -kpad, left, center, right, +keypad, end, down, pgdn, ins, del */
{	0x4700,	0x37,	0x7700,	0,	0x37,	0x4700,	0x37,		0x4700},
{	0x4800,	0x38,	0,	0,	0x38,	0x4800,	0x38,		0x4800},
{	0x4900,	0x39,	0x8400,	0,	0x39,	0x4900,	0x39,		0x4900},
{	0x2D,	0x2D,	0,	0,	0x2D,	0x2D,	0x2D,		0x2D},
{	0x4B00,	0x34,	0x7300,	0,	0x34,	0x4B00,	0x34,		0x4B00},
{	0x4C00,	0x35,	0,	0,	0x35,	0x4C00,	0x35,		0x4C00},
{	0x4D00,	0x36,	0x7400,	0,	0x36,	0x4D00,	0x36,		0x4D00},
{	0x2B,	0x2B,	0,	0,	0x2B,	0x2B,	0x2B,		0x2B},
{	0x4F00,	0x31,	0x7500,	0,	0x31,	0x4F00,	0x31,		0x4F00},
{	0x5000,	0x32,	0,	0,	0x32,	0x5000,	0x32,		0x5000},
{	0x5100,	0x33,	0x7600,	0,	0x33,	0x5100,	0x33,		0x5100},
{	0x5200,	0x30,	0,	0,	0x30,	0x5200,	0x30,		0x5200},
{	0x5300,	0x2E,	0,	0,	0x2E,	0x5300,	0x2E,		0x5300}
};


//////////////////////////////////////////////////////////////////////////////
// do the initialization of the keyboard                                    //
//////////////////////////////////////////////////////////////////////////////
void initialize_keyboard(void)
{
  start_status("Initializing keyboard");

  keyevent_handler = NULL;

  ivm_setirq(KEYBOARD_INTERRUPT,
             isr_keyboard_handler, STANDARD_INTERRUPT);

  enable_irq(KEYBOARD_INTERRUPT);

#if KB_NUMLOCK_ON
  LEDstatus |= LED_NUM_LOCK;
#endif

  update_leds();
  update_status(STATUS_OK);
}

//////////////////////////////////////////////////////////////////////////////
// update status of LEDs on keyboard                                        //
//////////////////////////////////////////////////////////////////////////////
void update_leds(void)
{
  outportb(0x60, 0xED);
  while(inportb(0x64) & 2) ;
  outportb(0x60, LEDstatus);
  while(inportb(0x64) & 2) ;
}

void install_keyhandler(void *ptr)
{
  keyevent_handler = ptr;
}

//////////////////////////////////////////////////////////////////////////////
// the actual C code keyboard handler                                       //
//////////////////////////////////////////////////////////////////////////////
INTERRUPT_STUB(keyboard_handler)
{
  byte key = inportb(0x60);
  u16i ascii = 0;
  struct keyb_event thisevent;

  if ( key == SCAN_NUM_LOCK ) // toggle numlock LED
     { LEDstatus ^= LED_NUM_LOCK; update_leds(); }
  if ( key == SCAN_SCROLL_LOCK ) // toggle numlock LED
     { LEDstatus ^= LED_SCROLL_LOCK; update_leds(); }
  if ( key == SCAN_CAPS_LOCK ) // toggle numlock LED
     { LEDstatus ^= LED_CAPS_LOCK; update_leds(); }

  if ( key == DEPRESSED(SCAN_CTRL) )  // if CTRL depressed
     controlkeys |= CK_CTRL;           // set control status
  if ( key == RELEASED(SCAN_CTRL) )   // if CTRL released
     controlkeys &= ~(CK_CTRL);        // set control status
  if ( key == DEPRESSED(SCAN_ALT) )   // if ALT depressed
     controlkeys |= CK_ALT;            // set control status
  if ( key == RELEASED(SCAN_ALT) )    // if ALT released
     controlkeys &= ~(CK_ALT);         // set control status
  if ( (key == DEPRESSED(SCAN_LSHIFT)) || (key == DEPRESSED(SCAN_RSHIFT)) )
     controlkeys |= CK_SHIFT;           // set control status
  if ( (key == RELEASED(SCAN_LSHIFT)) || (key == RELEASED(SCAN_RSHIFT)) )
     controlkeys &= ~(CK_SHIFT);        // set control status

#if ( KB_DEBUG_SCANS )
  for ( ascii = SCAN_BUFSIZE-1; ascii > 0; ascii-- )
    scanbuffer[ascii] = scanbuffer[ascii-1];

  scanbuffer[0] = key;
  print_controlstatus(key);
#endif

  if ( (controlkeys & (CK_CTRL|CK_ALT)) == (CK_CTRL|CK_ALT) ) // CTRL+ALT combination
     {
     ServiceSystemKey(key);       // let system handle this
     outportb(MASTER_PIC, EOI);  // don't process further
     return;
     }

  ascii = getasciicode(key);

  thisevent.ctrlstate = LEDstatus | controlkeys;
  thisevent.scancode  = key;
  thisevent.asciicode = ascii;
  thisevent.extcode   = ascii >> 8;

  if ( keyevent_handler != NULL ) // call the shell event handler
     keyevent_handler(thisevent);

  if ( ascii != 0 && PRESS(key) ) // if didn't return 0x0000 and not a release
     {
     if ( (byte)ascii == 0 ) // if low byte is 0
        {
        queuekey(0);        // put 0 in the buffer
        queuekey(ascii>>8); // put extended code in buffer
        }
     else queuekey(ascii);  // otherwise just one byte in buffer
    }

  outportb(MASTER_PIC, EOI); // tell pic we're done
}

//////////////////////////////////////////////////////////////////////////////
//  use internally to put key into getch buffer                             //
//////////////////////////////////////////////////////////////////////////////
void queuekey(byte key)
{
  if ( kb_bufsize < (KB_BUFFER_SIZE-1) ) // if buffer not full
     {
     keybuffer[kb_bufsize] = key;        // put in buffer
     kb_bufsize++;                       // advance buffer size
     }
}

//////////////////////////////////////////////////////////////////////////////
// debugging function to see what keys do                                   //
//////////////////////////////////////////////////////////////////////////////
#if ( KB_DEBUG_SCANS )
void print_controlstatus(byte key)
{
  u16i xy;
  byte cntr;

  xy = getcursorpos();

  movecursor(0,0);

  if ( controlkeys & CK_SHIFT ) printstring("SHIFT ", LIGHTBLUE);
  else printstring("      ", TEXT_NORMAL);
  if ( controlkeys & CK_CTRL ) printstring("CTRL ", LIGHTBLUE);
  else printstring("     ", TEXT_NORMAL);
  if ( controlkeys & CK_ALT ) printstring("ALT ", LIGHTBLUE);
  else printstring("    ", TEXT_NORMAL);

  if ( key & 0x80 ) { printstring("UP: ", TEXT_NORMAL); key &= ~(0x80); }
  else printstring("DN: ", TEXT_NORMAL);

  printhex(key, TEXT_HILIGHT);

  printstring(" ASCII: ", TEXT_NORMAL);

  cntr = getasciicode(key);

  if ( cntr > 32 ) putch(cntr, TEXT_HILIGHT);
  else putch(' ', TEXT_HILIGHT);

  for ( cntr = 0; cntr < SCAN_BUFSIZE; cntr++ )
     {
     printstring(" ", TEXT_NORMAL);
     printhex(scanbuffer[cntr], TEXT_NORMAL);
     }

  movecursor(xy&0xff, xy>>8);
}
#endif

//////////////////////////////////////////////////////////////////////////////
//  print hexidecimal byte                                                  //
//////////////////////////////////////////////////////////////////////////////
void printhex(byte c, byte attr)
{
  putch(hexdigits[c>>4], attr);
  putch(hexdigits[c&0xf], attr);
}

//////////////////////////////////////////////////////////////////////////////
// get a key from the buffer, if none, wait for one                         //
//////////////////////////////////////////////////////////////////////////////
byte getch(void)
{
  byte result;
  u16i cntr;

  while ( kb_bufsize == 0 ) ; // wait till something in the buffer

  result = keybuffer[0]; // get first key in the buffer
  kb_bufsize--;          // decrement used buffer space

  for ( cntr = 0; cntr < KB_BUFFER_SIZE-1; cntr++ ) // move everything forward
     keybuffer[cntr] = keybuffer[cntr+1];

  return result; // return resulting key
}

//////////////////////////////////////////////////////////////////////////////
// get a string from the input                                              //
// returns true unless user presses ESC                                     //
//////////////////////////////////////////////////////////////////////////////
byte getstring(cstr s)
{
  byte ch;
  u16i cntr;
  u16i max = 100;

  for ( cntr = 0; ; cntr++ )
     {
     ch = getch();

     if ( cntr == max ) // buffer full
        { if ( ch != 13 && ch != 8 ) { cntr--; continue; } }

     if ( ch == 13 )            // user pressed enter
        { s[cntr] = 0; break; } // accept
     else if ( ch == 8 )
        {
        if ( cntr > 0 ) // if something in the buffer
           {
           printstring("\b \b", TEXT_NORMAL); // clear character on screen
           cntr--;
           }
        cntr--; // after cntr is incremented, it will be the same
        }
     else if ( ch == 27 ) // user pressed esc
        {
        for ( cntr = 0; cntr < max; cntr++ ) s[cntr] = 0; // clear buffer
        return 0;
        }
     else if ( ch == 0 ) // extended key
        {
        ch = getch();
        cntr--;
        }
     else if ( ch < 32 ) cntr--; // not ascii key
     else
        {
        s[cntr] = ch;            // default, insert into buffer
        putch(ch, TEXT_NORMAL);  // and display on the screen
        }
     }

  return 1;
}

//////////////////////////////////////////////////////////////////////////////
// returns true if anything is in the buffer                                //
//////////////////////////////////////////////////////////////////////////////
byte kbhit(void)
{
  if ( kb_bufsize ) return 1;

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//  used internally to translate scan into ascii value                      //
//////////////////////////////////////////////////////////////////////////////
u16i getasciicode(byte key)
{
  u16i ascii = 0;

  if ( RELEASE(key) ) key ^= 0x80;

	if((controlkeys & CK_SHIFT) && (LEDstatus & LED_CAPS_LOCK)) ascii = scan_table[key][6];
	else if((controlkeys & CK_SHIFT) && (LEDstatus & LED_NUM_LOCK)) ascii = scan_table[key][7];
	else if(controlkeys & CK_ALT) ascii = scan_table[key][3];
	else if(controlkeys & CK_CTRL) ascii = scan_table[key][2];
	else if(controlkeys & CK_SHIFT) ascii = scan_table[key][1];
	else if(LEDstatus & LED_CAPS_LOCK) ascii = scan_table[key][5];
	else if(LEDstatus & LED_NUM_LOCK) ascii = scan_table[key][4];
	else if(controlkeys == 0) ascii = scan_table[key][0];

	return ascii;
}

