//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-18-2001                                                      //
// Header:  ia32/cpu/cpustate_def.h                                         //
// Purpose: This header makes declarations of the members of the cpustate   //
//          structure needed by the IA-32 implementation.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

u32i cr0;                 // CR0 control register
u32i cr1;                 // CR1 control register
u32i cr2;                 // CR2 fault address register
u32i cr3;                 // CR3 page directory base register
u32i cr4;                 // CR4 control register
u32i eax, ebx, ecx, edx;  // general purpose registers
u32i esp, ebp, edi, esi;  // general purpose registers
u32i eip;                 // instruction pointer
u16i cs, ds;              // segment register
u32i errcode;
u32i eflags;
