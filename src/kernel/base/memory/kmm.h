//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-19-2000                                                       //
// Header:  base/memory/kmm.h                                               //
// Purpose: This portion of the kernel handles malloc() like memory         //
//          management, including the creation and deletion of memory       //
//          caches like the slab allocator in Solaris.                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_KMM_H_
#define _BASE_KMM_H_

#include <base/configure.h>
#include <base/kernel.h>

// kernel malloc block (head of an allocated block)
typedef struct _hblk_head
{
  word size;

  struct _hblk_head *next;

} hblk_head __attribute__ ((packed));

// kernel malloc block (head of a free block)
//   the extra "nextf" member is used to maintain
//   the free linked list.
typedef struct _fblk_head
{
  word size;

  hblk_head *next;
  struct _fblk_head *nextf;

} fblk_head __attribute__ ((packed));

typedef struct _heap
{
  word startpage; // starting page
  word lastpage;  // last page in current heap
  word endpage;   // maximum address

} heap __attribute__ ((packed));

// initialization functions
void kmm_init(void);

// malloc/free routines for kernel
void *kmm_alloc(word size);
void kmm_free(void *ptr);

cache *kmm_createcache(word size);
void  *kmm_alloccache(cache *c);

// debugging functions
hblk_head *kmm_getfirstblock(void);
fblk_head **kmm_getfreelists(void);

#endif
