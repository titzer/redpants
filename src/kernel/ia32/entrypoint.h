//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-13-2000                                                       //
// Header:  ia32/entrypoint.h                                               //
// Purpose: This header contains the machine specific entry point of the    //
//          kernel, which then calls main().                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_ENTRYPOINT_H_
#define _IA32_ENTRYPOINT_H_

#ifdef _KERNEL_ANCHOR_ // this stuff should only be included once

int main(void);


//////////////////////////////////////////////////////////////////////////////
// entrypoint into the kernel: init stack and GDT, call main                //
//////////////////////////////////////////////////////////////////////////////
void _start(void)
{
  word stackbegin = _640kb - 4; // point to stack just before 640kb mark

  asm (
       " pushl $2            \n"
       " popf                \n" // zero all the flags
       " movl %0, %%esp      \n" // set up kernel stack
       :
       : "r" (stackbegin)
       );

  cpu_init();         // init the GDT
  main();             // call the main function now
}

#endif // _KERNEL_ANCHOR_
#endif // _ENTRYPOINT_H_
