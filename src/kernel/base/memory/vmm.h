//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-19-2000                                                      //
// Header:  base/memory/vmm.h                                               //
// Purpose: This header contains the declarations of the objects that       //
//          the virtual memory manager uses.                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_VMM_H_
#define _BASE_VMM_H_

#include <base/configure.h>
#include <base/cpu/task.h>
#include <base/memory/memory.h>

#define PAGE_SIZE _4kb

// Each physical page has a status associated with it (not formally
// stored). The virtual memory manager first attempts to allocate pages
// from the free list, then tries from the unmapped (but replicated
// on disk) list. If there are no pages in either of these available,
// the vmm will block the process allocating until the swap daemon makes
// unmapped pages available.
#define PSTATUS_FREE      0
#define PSTATUS_UNMAPPED  1
#define PSTATUS_DIRTY     2
#define PSTATUS_MAPPED    3

void vmm_init(void);
void vmm_startswapper(void);

// page allocation/deallocation functions
word vmm_allocatepage(word type);
word vmm_freepage(word pgnum);
void vmm_copypage(word pgsrc, word pgdest);
void vmm_zeropage(word pgsrc);
void vmm_copyphysicalpage(word pgsrc, word pgdest);
word vmm_extendheap(heap *hp, word pages);

// swap daemon functions
word vmmsd_allocatepage(void);
word vmmsd_removepage(word pgnum);
word vmmsd_retrievepage(word tasknum, word pgnum);

#endif
