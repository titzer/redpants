//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-19-2001                                                       //
// Module:  cpu/cpu.c                                                       //
// Purpose: This module contains the start up code that detects and init-   //
//          ializes the processors. It also provides some useful functions  //
//          for other parts of the kernel that need to do things like       //
//          enable/disable interrupts and identify which processor they are //
//          currently running on.                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <ia32/cpu/desctables.h>
#include <ia32/cpu/cpu.h>
#include <ia32/cpu/rdtsc.h>
#include <base/cpu/task.h>

#define STR(x) #x

//////////////////////////////////////////////////////////////////////////////
// the GDT needed by the x86 architecture processors                        //
//////////////////////////////////////////////////////////////////////////////
desc_table(gdt_table, 3)
{
  {dummy: 0 },
  stnd_desc(0, 0xFFFFF, (D_CODE + D_READ + D_4KPAGE + D_32BIT)),
  stnd_desc(0, 0xFFFFF, (D_DATA + D_WRITE + D_4KPAGE + D_32BIT)),
};

//////////////////////////////////////////////////////////////////////////////
// the structure needed for the LGDT instruction                            //
//////////////////////////////////////////////////////////////////////////////
struct
{
  unsigned short limit __attribute__ ((packed));
  union DT_entry *gdt __attribute__ ((packed));
  
} loadgdt = { (3 * sizeof(union DT_entry) - 1), gdt_table };


//////////////////////////////////////////////////////////////////////////////
// initialize the primary processor and detect its features.                //
//////////////////////////////////////////////////////////////////////////////
void cpu_init(void)
{
  asm volatile
     (
     "lgdtl (loadgdt)            \n" // load kernel GDT
     "movw $"STR(KERNEL_DS)", %%ax     \n" // initialize segment selectors
     "movw %%ax,          %%ds     \n" // DS
     "movw %%ax,          %%es     \n" // ES
     "movw %%ax,          %%fs     \n" // FS
     "movw %%ax,          %%gs     \n" // GS
     "movw %%ax,          %%ss     \n" // SS
     "ljmp $0x8,$next   \n" // do the jump to flush the prefetch queque
     "nop                        \n"
     "nop                        \n"
     "next:                      \n"
     :
     :
     : "%eax"
     );
}

//////////////////////////////////////////////////////////////////////////////
// determine which cpu we are currently running on.                         //
//////////////////////////////////////////////////////////////////////////////
word cpu_which(void)
{
#if SMP_SUPPORT
  return 0;
#else
  return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// disable interrupts on the current cpu and return status word.            //
//////////////////////////////////////////////////////////////////////////////
word cpu_disable_interrupts(void)
{
  word ps;

  asm (
       " pushf      \n"
       " popl %%eax \n"
       " cli        \n"
       : "=a" (ps) );

  return ps;
}

//////////////////////////////////////////////////////////////////////////////
// restore interrupts to a previously saved status.                         //
//////////////////////////////////////////////////////////////////////////////
void cpu_restore_interrupts(word w)
{
  asm (
       " pushl %%eax  \n"
       " popf         \n"
       :
       : "a" (w)
	);
}

//////////////////////////////////////////////////////////////////////////////
// context switch to another task.                                          //
//////////////////////////////////////////////////////////////////////////////
void cpu_ctxsw(cpustate *s)
{
}

u64i cpu_cycles(void)
{
  u64i result;

  if ( !rdtsc_supported() ) return 0;

  rdtsc(&result);
  return result;
}
