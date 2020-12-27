//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    2-17-2000                                                       //
// Module:  swap.c                                                          //
// Purpose: This is the swap daemon process that handles selecting pages    //
//          from memory to be paged out by approximating the least recently //
//          used pages.                                                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/configure.h>

#define DEBUG_SWITCH DEBUG_SWAP
#define DEBUG_MODULE "SWAP"
#define DEBUG_COLOR  DEBUG_SWAP_COLOR

#include <base/memory/memory.h>
#include <base/memory/vmm.h>
#include <base/kernel.h>

// local functions
void vmmsd_entrypoint(void);
void vmmsd_writedirtypages(word num);

// local data
word pst_pos;             // current position in the page status table
word bufferhead;          // head of pageout buffer
word buffertail;          // tail of the pageout buffer
word buffermax;           // maximum (goal) size of buffer
word numclean, numdirty;  // number of clean and dirty pages

//////////////////////////////////////////////////////////////////////////////
// vmm_startswapper(void)                                                   //
//                                                                          //
//  Start up the VMM's swap daemon that decides which pages are least       //
// recently used and are to be paged out when demand for free pages is high //
//////////////////////////////////////////////////////////////////////////////
void vmm_startswapper(void)
{
  numclean = numdirty = 0;                // nothing in the buffer to start
  bufferhead = buffertail = 0;            // initialize head/tail of list
  buffermax = kmm_getphysicalsize() * 32; // 32 pages per meg
  pst_pos = 1;                            // start at page 1

}

//////////////////////////////////////////////////////////////////////////////
//  Entrypoint of the swap daemon process                                   //
//////////////////////////////////////////////////////////////////////////////
void vmmsd_entrypoint(void)
{
}

//////////////////////////////////////////////////////////////////////////////
//  Removes a page from the page out buffer, whether dirty or clean         //
//////////////////////////////////////////////////////////////////////////////
word vmmsd_removepage(word pgnum)
{
  word prev, next;

  if ( pgnum >= physpages ) return 0; // invalid page num
  if ( !pst[pgnum].inpob ) return 0;  // the INPageOutBuffer bit is not on

  prev = pst[pgnum].prev; // get previous pointer
  next = pst[pgnum].next; // get next pointer

  if ( prev ) pst[prev].next = next; // patch previous
  if ( next ) pst[next].prev = prev; // patch next

  if ( pst[pgnum].dirty ) numdirty--; // lost a dirty page
  else numclean--;                    // lost a clean page

  pst[pgnum].inpob = 0; // this page no longer in page out buffer
  return pgnum;
}

//////////////////////////////////////////////////////////////////////////////
//  Allocate a page from the oldest page in buffer                          //
//////////////////////////////////////////////////////////////////////////////
word vmmsd_allocatepage(void)
{
  word pgcntr;

  for ( pgcntr = bufferhead; pgcntr != 0; pgcntr = pst[pgcntr].next )
     {
     if ( !pst[pgcntr].dirty ) // clean page, we can allocate it
        {
        vmmsd_removepage(pgcntr);
        break;
        }
     }

  return pgcntr;
}

//////////////////////////////////////////////////////////////////////////////
//  Seek more page out candidates and put them into the buffer              //
//////////////////////////////////////////////////////////////////////////////
void vmmsd_seekpages(word num)
{
  word start_pos = pst_pos;

  while ( num > 0 )
     {
     if ( pst[pst_pos].pageable && pst[pst_pos].mapped ) // candidate found
        {
        // check if accessed in page table. if not,
        // then remove from page table and then
        // put it into buffer
        num--;
        }

     pst_pos++;
     if ( pst_pos == physpages ) pst_pos = 1; // wrap around to page 1
     if ( pst_pos == start_pos ) break;       // tried all of physical memory
     }
}

//////////////////////////////////////////////////////////////////////////////
//  Write num oldest dirty pages to disk                                    //
//////////////////////////////////////////////////////////////////////////////
void vmmsd_writedirtypages(word num)
{
}

//////////////////////////////////////////////////////////////////////////////
//  Retrieve a page from backing store                                      //
//////////////////////////////////////////////////////////////////////////////
word vmmsd_retrievepage(word tasknum, word pgnum)
{
  return 0;
}
