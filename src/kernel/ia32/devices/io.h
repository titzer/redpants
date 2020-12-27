//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    9-17-2001                                                       //
// Header:  ia32/devices/io.h                                               //
// Purpose: This header contains declarations of functions that access the  //
//          IO ports of the PCI bus. These ports are needed by most devices //
//          and in some parts of the kernel.                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_IO_H_
#define _IA32_IO_H_

// function headers to input/outputting to a port
inline byte  inportb  (u32i port);
inline void outportb  (u32i port, byte value);
inline void outportw  (u32i port, u32i value);
inline byte  inportb_p(u32i port);
inline void outportb_p(u32i port, byte value);
inline void outportw_p(u32i port, u32i value);


#ifdef _KERNEL_ANCHOR_

// function definitions for inputting/outputting to a port

inline byte inportb_p(u32i port)
{
  byte ret;
  asm volatile ("inb %%dx,%%al;outb %%al,$0x80":"=a" (ret):"d" (port));
  return ret;
}

inline byte inportb(u32i port)
{
   byte ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

inline void outportb_p(u32i port,byte value)
{
  asm volatile
     ("outb %%al,%%dx;outb %%al,$0x80": : "d" (port), "a" (value));
}

inline void outportw_p(u32i port,u32i value)
{
  asm volatile
    ("outw %%ax,%%dx;outb %%al,$0x80": :"d" (port), "a" (value));
 }

inline void outportb(u32i port,byte value)
{
  asm volatile
     ("outb %%al,%%dx": :"d" (port), "a" (value));
}

inline void outportw(u32i port,u32i value)
{
  asm volatile
     ("outw %%ax,%%dx": :"d" (port), "a" (value));
}

#endif
#endif
