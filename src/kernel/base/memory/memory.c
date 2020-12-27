//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-23-2000                                                       //
// Module:  memory.c (formerly memmap.c)                                    //
// Purpose: This portion of the kernel handles initializing and accessing   //
//          physical memory, and coordinates the initializing of the        //
//          various memory management facilities of the kernel.             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/configure.h>

#define DEBUG_SWITCH DEBUG_MEM_INIT
#define DEBUG_MODULE "MEM"
#define DEBUG_COLOR  DEBUG_MEM_COLOR

#include <base/memory/memory.h>
#include <base/memory/vmm.h>
#include <base/util/debug.h>

// memory counting constants--just random values, really
// we test if memory is there by writing the value there
// and then trying to read it back to see if it was stored
#define TEST_VALUE 0x1CDE09AB
#define TEST_MEMPOS_1 0xff7f0
#define TEST_MEMPOS_2 0x31ff0


// local functions
void measure_physical(void);

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//     The virtual memory manager is designed so that the virtual address   //
//  space provided by the processor is broken up into a user and a system   //
//  portion. By the term virtual address space, the address space that the  //
//  application sees and can access is intended. The reason for dividing    //
//  an application's virtual address space into a user and a system portion //
//  is so that interrupts/exceptions can be handled by the system code in   //
//  the system portion of the address space without causing a complete      //
//  address space switch, which is pretty expensive in terms of CPU cycles. //
//     The user code of the application is not permitted to read, modify,   //
//  or even address the system portion of the address space. It is kept     //
//  from doing so via segmentation. The user code and data segments do not  //
//  overlap any system memory and therefore cannot corrupt it.              //
//     The resulting map of virtual memory looks something like this:       //
//                                                                          //
//  |--- kernel / system --|----------------- user -------------------|     //
//  0      . . . . .      1gb              . . . . .                 4gb    //
//                                                                          //
//     The first gigabyte of the virtual address space is used by the       //
//  system code and data and remains constant across all application's      //
//  virtual address spaces. The system code then handles all exceptions,    //
//  interrupts, and system calls within the virtual address space of the    //
//  application. Though applications on the same system may have wildly     //
//  different user address spaces, the system address spaces translate      //
//  to the same physical memory, ensuring consistent system behavior no     //
//  matter what virtual address space the processor is operating with.      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// private memory management data
word physmem = 0;

//////////////////////////////////////////////////////////////////////////////
// initialize the memory system                                             //
//////////////////////////////////////////////////////////////////////////////
void initialize_memory(void)
{

  DEBUG_PRINT("Measuring physical memory...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  measure_physical();       // measure physical memory
  DEBUG_INT(physmem);
  DEBUG_STR("mb");
  DEBUG_DEL(DEBUG_STD_DELAY);

  DEBUG_PRINT("Initializing virtual memory manager (VMM)...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  //  initialize_vmm();        // initialize the virtual memory manager

  DEBUG_PRINT("Initializing kernel memory manager (KMM)...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  //  initialize_kmm();   // initialize the kernel memory manager
}

//////////////////////////////////////////////////////////////////////////////
// measure physical memory                                                  //
//////////////////////////////////////////////////////////////////////////////
void measure_physical(void)
{
  word megs = 1;
  word tmp;
  word cntr;
  wptr padd; // physical address

  for ( cntr = 1; cntr != 0; cntr++ ) // loop each megabyte
     {
     padd = (wptr)((cntr<<20)+TEST_MEMPOS_1);
     tmp = *padd;                      // save what is there
     *padd = TEST_VALUE;               // try to write to memory
     asm("": : :"memory");             // empty asm to get around optimization
     if ( *padd != TEST_VALUE ) break; // if not successful, break
     *padd = tmp;                      // restore what was there

     padd = (wptr)((cntr<<20)+TEST_MEMPOS_2);
     tmp = *padd;                      // save what is there
     *padd = TEST_VALUE;               // try to write to memory
     asm("": : :"memory");             // empty asm to get around optimization
     if ( *padd != TEST_VALUE ) break; // if not successful, break
     *padd = tmp;                      // restore what was there

     megs++;                           // increment
     }

  physmem = system_info.physical_memory = megs;  // store result
}


word kmm_getphysicalsize(void)
{
  return physmem;
}
