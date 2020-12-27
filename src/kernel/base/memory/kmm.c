//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-28-2000                                                       //
// Module:  base/memory/kmm.c                                               //
// Purpose: This portion of the kernel handles memory management for the    //
//          kernel internally.                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/memory/memory.h>
#include <base/memory/vmm.h>

#define DEBUG_SWITCH DEBUG_KMM
#define DEBUG_MODULE "KMM"
#define DEBUG_COLOR  DEBUG_KMM_COLOR

#include <base/util/debug.h>

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Notes:                                                                  //
//                                                                          //
//    This kernel memory manager uses a hybrid buddy system/first fit       //
//  allocation routine. The buddy system (used by Linux and Windows NT)     //
//  is very fast, but wastes memory because blocks can only be allocated    //
//  in powers of two. The first fit algorithm is more efficient in terms    //
//  of memory usage, but is slower.                                         //
//                                                                          //
//    This hybrid approach retains the best of both worlds. By classifying  //
//  free blocks (that can be any size) into groups based on their size,     //
//  it eliminates the need to search the entire free block list looking     //
//  for a block large enough to do the allocation. When a block is found    //
//  that is large enough, it is split into a block the correct size and a   //
//  leftover block that is reinserted into the free list of the correct     //
//  class. This allows memory to be used as efficiently as it is in the     //
//  first fit system, and the classification mechanism narrows down the     //
//  searching considerably, resulting in much faster searches.              //
//                                                                          //
//    The entire heap (including allocated blocks) is maintained through    //
//  a linked list that is part of the header of each block. The free lists  //
//  are maintained by a second linked list that is only part of the header  //
//  of a free block. This means that the headers of allocated blocks and    //
//  free blocks are not the same size. The convention in the free list is   //
//  to report the allocatable portion of the block as if it the extra link  //
//  did not take up any space. Basically, the extra link memory in free     //
//  blocks is treated like it doesnt exist, and the size field is kept      //
//  consistent between allocated and free blocks.                           //
//                                                                          //
//    Currently blocks are allocated in 32 bit aligned sizes, so that the   //
//  least 2 significant bits of the size field are always zero. The least   //
//  significant bit of the size field is used indicate whether or not a     //
//  block is currently allocated, 0 if free, 1 if allocated. This is needed //
//  in attempting to coalesce blocks because the main linked list always    //
//  maintains an in-order list of blocks. By checking the allocated bit of  //
//  the next block in the list when freeing a particular block, it can be   //
//  determined whether it is possible to merge the two.                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define USED_BIT BIT(0)

#define HBLK_HEAD_SIZE sizeof(hblk_head)
#define FBLK_HEAD_SIZE sizeof(fblk_head)

byte get_class(word size);
void insert_freeblock(fblk_head *ptr);
void remove_freeblock(fblk_head *ptr);
word init_baseheap(void);

byte kmm_initialized = no; // don't allow initialization more than once

fblk_head *free_lists[KMM_HEAP_CLASSES]; // free lists
hblk_head *main_block; // points to main heap memory block

//////////////////////////////////////////////////////////////////////////////
// initialize_kmm:                                                          //
//   initialize the kernel memory manager                                   //
//////////////////////////////////////////////////////////////////////////////
void initialize_kmm(void)
{
  u32i heap_pages = 0;

  if ( kmm_initialized ) // make sure we only initialize once
     {
     DEBUG_PRINT("KMM already initialized.");
     return;
     }

  DEBUG_PRINT("Initializing kernel space memory heap...");
  DEBUG_DEL(DEBUG_STD_DELAY);

  heap_pages = init_baseheap();

  DEBUG_PRINT("Initial heap size: ");
  DEBUG_INT(heap_pages);
  DEBUG_STR(" pages");
  DEBUG_PRINT("Total free memory for heap (KB): ");
  DEBUG_INT(heap_pages*PAGE_SIZE/_1kb);
  DEBUG_DEL(DEBUG_STD_DELAY);

  kmm_initialized = yes; // we are done
}

//////////////////////////////////////////////////////////////////////////////
// init_baseheap:                                                           //
//   initialize the base part of the heap                                   //
//////////////////////////////////////////////////////////////////////////////
word init_baseheap(void)
{
  word pages, size;

  DEBUG_PRINT("Heap (start, last, max) page: (");
  DEBUG_H16(sys_heap->startpage);
  DEBUG_STR(", ");
  DEBUG_H16(sys_heap->lastpage);
  DEBUG_STR(", ");
  DEBUG_H16(sys_heap->endpage);
  DEBUG_STR(")");
  DEBUG_DEL(DEBUG_STD_DELAY);

  DEBUG_PRINT("Extending heap by ");
  DEBUG_INT(KMM_EXTEND_SIZE*PAGE_SIZE/_1kb);
  DEBUG_STR("kb...");
  DEBUG_DEL(DEBUG_STD_DELAY);

  pages = vmm_extendheap(sys_heap, KMM_EXTEND_SIZE);

  size = pages*PAGE_SIZE;

  DEBUG_PRINT("Heap (start, last, max) page: (");
  DEBUG_H16(sys_heap->startpage);
  DEBUG_STR(", ");
  DEBUG_H16(sys_heap->lastpage);
  DEBUG_STR(", ");
  DEBUG_H16(sys_heap->endpage);
  DEBUG_STR(")");
  DEBUG_DEL(DEBUG_STD_DELAY);

  main_block = (hblk_head *)(sys_heap->startpage*PAGE_SIZE);
  main_block->next = 0;
  main_block->size = size - HBLK_HEAD_SIZE;

  DEBUG_PRINT("Inserting free block...");
  DEBUG_DEL(DEBUG_STD_DELAY);

  insert_freeblock((fblk_head *)main_block);

  return pages;
}

//////////////////////////////////////////////////////////////////////////////
// kmm_alloc:                                                               //
//   allocate a memory block in the kernel address space                    //
//////////////////////////////////////////////////////////////////////////////
void *kmm_alloc(word size)
{
  word cl,
       realsize,
       leftover;

  void *ptr;
  fblk_head *fprev = NULL; // previous, for faster removing
  fblk_head *fptr  = NULL; // current free block
  hblk_head *hptr;  // pointer to heapblock structure

  size = 1 + (size-1)/KMM_MALLOC_ALIGN;
  size *= KMM_MALLOC_ALIGN;

  realsize = size + HBLK_HEAD_SIZE; // get needed space

  cl = get_class(realsize); // get requested size class

  DEBUG_PRINT("Allocation request: ");
  DEBUG_INT(size);
  DEBUG_STR(" + ");
  DEBUG_INT(HBLK_HEAD_SIZE);
  DEBUG_STR(", class ");
  DEBUG_INT(cl);
  DEBUG_STR(": ");

  while ( cl < KMM_HEAP_CLASSES ) // loop up to highest size class
     {
     fptr = fprev = free_lists[cl]; // point to first block in free list class

     while ( fptr != NULL ) // search through free blocks
        {
        if ( fptr->size >= realsize ) break; // we have got a match

        fprev = fptr;
        fptr = fptr->nextf; // go to next free block
        }

     if ( fptr != NULL ) break; // found one, break out of loop
     cl++; // otherwise, continue to next highest class
     }

  if ( fptr == NULL ) // failed to find any free block
     {
     DEBUG_STR("failed");
     return NULL;
     }

  leftover = fptr->size - realsize; // leftover space from free block

  if ( leftover < (FBLK_HEAD_SIZE+KMM_MALLOC_ALIGN ) ) // if not enough left
     {                                                  // remove entire block
     if ( fprev == fptr ) // if first in free list
        free_lists[cl] = fptr->nextf; // point start of free list to next
     else fprev->nextf = fptr->nextf; // otherwise, skip this node

     hptr = (hblk_head *)fptr; // use whole block
     }
  else
     {
     ptr = ((void *)fptr) + fptr->size - realsize; // point to new block
     hptr = (hblk_head *)ptr;               // cast
     hptr->size = realsize | USED_BIT;             // set size, and signal used
     hptr->next = fptr->next;                      // insert in block chain
     fptr->next = hptr;
     fptr->size -= (realsize + HBLK_HEAD_SIZE);    // change size of original

     if ( get_class(fptr->size) != cl ) // if class changed, remove, reinsert
        {
        if ( fprev == fptr ) // if first in free list
           free_lists[cl] = fptr->nextf; // point start of free list to next
        else fprev->nextf = fptr->nextf; // otherwise, skip this node

        insert_freeblock(fptr); // reinsert into the free list
        }
     }


  DEBUG_STR("succeeded");
  return ((void*)hptr)+HBLK_HEAD_SIZE; // return memory portion
}

//////////////////////////////////////////////////////////////////////////////
// kernel_free:                                                             //
//   free a memory block in the kernel address space                        //
//////////////////////////////////////////////////////////////////////////////
void kmm_free(void *ptr)
{
  fblk_head *fptr;

  fptr = (fblk_head *)(ptr - HBLK_HEAD_SIZE);
  fptr->size &= ~(USED_BIT);  // clear used bit

  if ( (fptr->next != NULL) && !(fptr->next->size & USED_BIT) ) // if next one is free too
     {
     fptr->size += (fptr->next->size + HBLK_HEAD_SIZE); // coalesce
     remove_freeblock((fblk_head *)fptr->next);  // remove other
     fptr->next = fptr->next->next;
     }

  insert_freeblock(fptr);
}


//////////////////////////////////////////////////////////////////////////////
// insert_freeblock:                                                        //
//   insert a new free block into the free lists                            //
//////////////////////////////////////////////////////////////////////////////
void insert_freeblock(fblk_head *ptr)
{
  byte cl;

  cl = get_class(ptr->size);

  ptr->nextf = free_lists[cl];
  free_lists[cl] = ptr;
}

//////////////////////////////////////////////////////////////////////////////
// remove_freeblock:                                                        //
//   find and remove a free block from the free lists                       //
//////////////////////////////////////////////////////////////////////////////
void remove_freeblock(fblk_head *ptr)
{
  byte cl;
  fblk_head *fptr, *fprev;

  cl = get_class(ptr->size);

  fptr = fprev = free_lists[cl];

  while ( fptr != NULL ) // loop until all in that class are gone
     {
     if ( fptr == ptr ) // if we found it, remove it
        {
        if ( fprev == fptr ) // if first in free list
           free_lists[cl] = fptr->nextf; // point start of free list to next

        else fprev->nextf = fptr->nextf; // otherwise, skip this node
        break;
        }

     fprev = fptr;
     fptr = fptr->nextf; // go to next free block
     }

}

//////////////////////////////////////////////////////////////////////////////
// get_class:                                                               //
//   get the size class of a particular allocation request                  //
//////////////////////////////////////////////////////////////////////////////
byte get_class(word size)
{
  byte cl = 0;

  size = size >> 2;

  while ( (size = size >> 1) ) cl++;

  return cl;
}

hblk_head *kmm_getfirstblock(void)
{
  return main_block;
}

fblk_head **kmm_getfreelists(void)
{
  return free_lists;
}
