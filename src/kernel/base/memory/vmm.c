//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-20-2000                                                      //
// Module:  vmm.c                                                           //
// Purpose: This module contains most of the functions and code that        //
//          implement the virtual memory manager.                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#define DEBUG_SWITCH DEBUG_VMM
#define DEBUG_MODULE "VMM"
#define DEBUG_COLOR DEBUG_VMM_COLOR

#include <base/memory/vmm.h>
#include <base/kernel.h>
#include <base/util/debug.h>
#include <base/memory/memory.h>
#include <base/cpu/traps.h>
#include <base/cpu/task.h>

#define C_PAGE_COPY no

void initialize_statustable(void);
void initialize_paging(void);
void clear_pagetable(pagetable);
void enable_paging(pagetable);
void build_freelist(void);
void handle_invalidpagefault(word type, pfparams *pfp);
pstatus_entry *map_pstintolinear(pagetable pd);

TRAP_DECLARATION(trap_page_fault);

#if ( DEBUG )
  #define DEBUG_PAGE_FAULT(a, b) debug_page_fault(a, b)
  void debug_page_fault(word type, pfparams *pfp);
#else
  #define DEBUG_PAGE_FAULT(a, b)
#endif

word sys_pdirnum;      // physical page number of page directory
heap _sys_heap;        // system heap structure
word pst_pages;        // size of page status table in KB

pagetable sys_pagedir;      // system (kernel) page directory (linear addr)
pagetable local_pagedir;    // pointer to local page directory (linear addr)
pagetable local_pagetables; // local pagetables (linear addr)

pstatus_entry default_page  = { 0, 0, 0, PTYPE_FREE, 0, 0, 0, 0, 0 },
                                  pst_page = { 0, 0, 0, PTYPE_STATUS, 0, 1, 0, 0, 0 };

word cpu_features;

word numpages[4]; // number of pages in each state
word listhead[4]; // heads of the lists of pages in each state

//////////////////////////////////////////////////////////////////////////////
// initialize_vmm()                                                         //
//                                                                          //
//  Initializes the virtual memory manager. Initialize memory pools, map    //
//  virtual address space for the kernel, initiate paging.                  //
//////////////////////////////////////////////////////////////////////////////
void initialize_vmm(void)
{
  DEBUG_PRINT("Initializing memory status table...");
  DEBUG_DELAY(1);

  sys_heap = &_sys_heap;
  
  initialize_statustable();

  initialize_paging();
}

//////////////////////////////////////////////////////////////////////////////
// initialize_statustable()                                                 //
//                                                                          //
//  Initializes the status table which stores the status of every page of   //
//  physical memory as well as its mapping in some address space.         //
//////////////////////////////////////////////////////////////////////////////
void initialize_statustable(void)
{
  word megs, cntr, pststart;
  pstatus_entry pse;

  megs = kmm_getphysicalsize(); // get physical size from kernel memory manager
  physpages = megs*(_1mb/PAGE_SIZE);
  cntr = physpages * sizeof(pstatus_entry); // find out how many pages the
  pst_pages = 1+(cntr-1)/PAGE_SIZE;         // actual PST occupies

// if <= 16mb physical, put page status table in page 1
  if ( megs <= 16 ) pst = (pstatus_entry *)PAGE_SIZE;
// otherwise put just after 16mb because we don't want to use up DMA-able
// memory to put the status table in. If we have 32 bit DMA, then it doesn't
// really matter either way, so 16mb is as good as any place.
  else pst = (pstatus_entry *)(_16mb);

  DEBUG_PRINT("Physical memory: ");
  DEBUG_INT(megs);
  DEBUG_STR("mb, ");
  DEBUG_INT(physpages);
  DEBUG_STR(" pages");
  DEBUG_PRINT("Page status table at: 0x");
  DEBUG_H32((word)pst);
  DEBUG_DELAY(1);

  numpages[PSTATUS_FREE] = 0; // all pages start out as free
  numpages[PSTATUS_UNMAPPED] = 0;
  numpages[PSTATUS_DIRTY] = 0;
  numpages[PSTATUS_MAPPED] = 0;

  for ( cntr = 0; cntr < physpages; cntr++ ) // clear the page status table
     pst[cntr] = default_page;

  pststart = ADDR_TO_PAGE(pst);             // get first pagenum of PST

  DEBUG_PRINT("Entry size: ");
  DEBUG_INT(sizeof(pstatus_entry)*8);
  DEBUG_STR(" bits, ");
  DEBUG_INT(pst_pages*(PAGE_SIZE/_1kb));
  DEBUG_STR("kb used ");
  DEBUG_DELAY(1);

  for ( cntr = 0; cntr < pst_pages; cntr++ ) // set PST entries for pages
     pst[cntr+pststart] = pst_page;          // occupied by PST itself

  pse.mapped = 1;       // now we setup this temp structure in order to
  pse.pageable = 0;     // set the status of several sections of memory
  pse.type = PTYPE_IO;
  pse.task = 0;
  pse.next = 0;

  pst[0] = pse; // first page is BIOS data area

  for ( cntr = 0xA0; cntr < 0x100; cntr++ ) // set 640k-1mb as IO
     pst[cntr] = pse;

  pse.type = PTYPE_KERNEL;     // set 1mb - 2mb as kernel code and data
  for ( cntr = 0x100; cntr < 0x200; cntr++ )
     pst[cntr] = pse;     // this just a rough estimate of the kernel's
                          // memory usage. later parts of memory in that
                          // range will be freed.

  pse.type = PTYPE_STACK;   // we use the page under 640k for system stack
  pst[0x9f] = pse;          // as well as the kernel task information
                            // see that in cpu/entrypoint.h, cpu/task.c

  build_freelist(); // build free list and count free pages

  DEBUG_PRINT("Free pages: ");
  DEBUG_INT(numpages[PSTATUS_FREE]);
  DEBUG_DELAY(1);

}

//////////////////////////////////////////////////////////////////////////////
// initialize_paging()                                                      //
//                                                                          //
//  Set up system page tables and initialize processor's paging features.   //
//////////////////////////////////////////////////////////////////////////////
void initialize_paging(void)
{
  word cntr, pgnum, pdirnum;
  pagetable pd;
  pagetable pt;
  pstatus_entry *pstlin = pst; // linear (new) address of PST

  cpu_features = system_info.cpu_features; // get cpu features

  DEBUG_PRINT("Allocating page directory...");
  pdirnum = pgnum = vmm_allocatesystempage(PTYPE_PDIR); // alloc page for page dir
  sys_pdirnum = pdirnum;
  pst[pdirnum].vpage = PTENUMS_TO_PAGE(SYS_PDIR, SYS_PDIR);
  DEBUG_STR("page #");
  DEBUG_H16(pgnum);
  DEBUG_DELAY(1);

  pd = (pagetable)PAGE_TO_ADDR(pgnum); // convert to pointer
  clear_pagetable(pd);

// point PD #255 (just under 1gb) to PDIR itself
// this allows us to access pagetables locally, automatically.
  pd[SYS_PDIR] = MAKE_PTE(pdirnum, PTE_STANDARD);
//  local_pagedir = (pagedir)(SYS_PDIR*PAGE_SIZE);

#if ( VMM_4MB_PAGES )
  if ( cpu_features & CPU_FEATURE_PSE )
     {
     DEBUG_PRINT("Enabling page size extensions...");
     DEBUG_DELAY(1);
     asm (
         " mov %cr4, %eax  \n" // get CR4
         " or  $16, %eax   \n" // set bit 4 (page size extensions)
         " mov %eax, %cr4  \n" // reset CR4
         );

// set the first page 0-4mb linear=physical using one 4mb page
     pd[0] = MAKE_PTE(0, PTE_STANDARD|PTE_4MBPAGE);
     }
  else
#endif
     {  // no 4mb page support, have to make a pagetable for 0-4mb
     DEBUG_PRINT("Allocating page table 0...");
     pgnum = vmm_allocatesystempage(PTYPE_PTABLE); // allocate page table
     pst[pgnum].vpage = PTENUMS_TO_PAGE(SYS_PDIR, 0);
     DEBUG_STR("page #");
     DEBUG_H16(pgnum);
     DEBUG_DELAY(1);

// allocate a page table for 0-4mb virtual addresses
     pt = (pagetable)PAGE_TO_ADDR(pgnum);          // cast
     clear_pagetable(pt);                          // clear it
     for ( cntr = 0; cntr < PTABLE_SIZE; cntr++ )  // point to 0-4mb physical
        pt[cntr] = MAKE_PTE(cntr, PTE_STANDARD);
     pd[0] = MAKE_PTE(pgnum, PTE_STANDARD);        // make PD point to it
     }

// get a page table for task local storage
// this is used for a pointer to the system page directory,
// the system stack, and the task information structures for threads
  DEBUG_PRINT("Allocating page table 254 (task local storage)...");
  pgnum = vmm_allocatesystempage(PTYPE_PTABLE); // allocate page table
  pt = (pagetable)PAGE_TO_ADDR(pgnum);          // cast page num to pagetable
  clear_pagetable(pt);                          // clear it
  pd[SYS_PLS] = MAKE_PTE(pgnum, PTE_STANDARD);      // point pd to it
  pst[pgnum].vpage = PTENUMS_TO_PAGE(SYS_PDIR, SYS_PLS);

// entry [254][1023] points to the system page directory
  pt[1023] = MAKE_PTE(pdirnum, PTE_STANDARD);   // point to actual PDIR

// entry [254][1022] points to process local storage and system stack
  pt[1022] = MAKE_PTE(ADDR_TO_PAGE(kernel_task), PTE_STANDARD);

// a nice little side effect of having pdir[255] point to pdir itself
// is that entry [255][255] also points to pdir itself, so we can address
// the local page directory, too.
  local_pagedir = (pagetable)(PTENUMS_TO_ADDR(SYS_PDIR, SYS_PDIR));
  local_pagetables = (pagetable)(PTENUMS_TO_ADDR(SYS_PDIR, 0));
  sys_pagedir = (pagetable)(PTENUMS_TO_ADDR(SYS_PLS, 1023));

  DEBUG_STR("page #");
  DEBUG_H16(pgnum);
  DEBUG_DELAY(1);
  kernel_task->pdirbase = pdirnum; // save the pagedir number in task data

  pstlin = map_pstintolinear(pd);
  DEBUG_PRINT("Linear address of PST: 0x");
  DEBUG_H32((word)pstlin);
  DEBUG_DELAY(1);
  DEBUG_PRINT("Linear address of system heapstart: 0x");
  DEBUG_H32(PAGE_TO_ADDR(sys_heap->startpage));
  DEBUG_DELAY(1);

  sys_heap->lastpage = sys_heap->startpage; // nothing allocated in heap yet
  sys_heap->endpage = ADDR_TO_PAGE(_512mb)-1; // go up to 512mb linear

// install the page fault handler
  DEBUG_PRINT("Installing page fault handler...");
  DEBUG_DELAY(1);
  set_vector(EXCPT_PAGE_FAULT, excpt_page_fault, STANDARD_EXCEPTION);

// set CR3 and enable paging on the processor
  DEBUG_PRINT("Enabling processor paging...");
  DEBUG_DELAY(1);
  enable_paging(pd); // enable paging with current page directory
  DEBUG_STR("enabled");

// now access PST through linear address
  pst = pstlin;
}

//////////////////////////////////////////////////////////////////////////////
//  map PST into linear address space of the OS.                            //
// also update the heapstart variable, since the heap starts                //
// immediately following the page status table.                             //
//////////////////////////////////////////////////////////////////////////////
pstatus_entry *map_pstintolinear(pagetable pd)
{
  word pst_startpage;
  word pst_4mbpages;
  word cntr;
  word pgnum;
  pagetable pt = 0;

  pst_startpage = (word)pst/PAGE_SIZE;

  DEBUG_PRINT("Mapping PST physical pages into linear address space...");
  DEBUG_DELAY(1);

  if ( pst_startpage < 1024 ) // if in first 4mb, already identity mapped
     {
     DEBUG_PRINT("PST already mapped in first 4mb at 0x");
     DEBUG_H32((word)pst);
     DEBUG_DELAY(1);
     sys_heap->startpage = ADDR_TO_PAGE(_4mb); // heap will then start after first 4mb
     return pst;
     }

#if ( VMM_4MB_PAGES )
  if ( cpu_features & CPU_FEATURE_PSE ) // make sure prcsr supports 4mb pages
     {
     DEBUG_PRINT("Using ");
     pst_4mbpages = 1+(pst_pages-1)/1024; // number of 4mb pages needed
     DEBUG_INT(pst_4mbpages);
     DEBUG_STR(" large (4mb) pages to map PST...");
     DEBUG_DELAY(1);

     for ( cntr = 0; cntr < pst_4mbpages; cntr++ )
        {
        pgnum = pst_startpage+cntr*1024;
        pd[cntr+1] = MAKE_PTE(pgnum, PTE_STANDARD|PTE_4MBPAGE);
        }

     // now the heap goes right after last 4mb PST page
     sys_heap->startpage = ADDR_TO_PAGE(_4mb*(pst_4mbpages+1));
     }
  else
#endif
     {
     DEBUG_PRINT("Using ");
     DEBUG_INT(pst_pages);
     DEBUG_STR(" small (4k) pages to map PST...");
     DEBUG_DELAY(1);

     for ( cntr = 0; cntr < pst_pages; cntr++ ) // map each PST page
        {
        if ( (cntr % PTABLE_SIZE) == 0 ) // need to allocate a new page table
           {
           DEBUG_PRINT("Allocating page table ");
           DEBUG_INT(cntr/PTABLE_SIZE);
           DEBUG_STR(" for PST mapping...");
           DEBUG_DELAY(1);
           pgnum = vmm_allocatesystempage(PTYPE_PTABLE); // allocate page table
           pt = (pagetable)PAGE_TO_ADDR(pgnum);          // cast page num to
           clear_pagetable(pt);                          // clear it
           pd[cntr+1] = MAKE_PTE(pgnum, PTE_STANDARD);
           }

        pt[cntr%PTABLE_SIZE] = MAKE_PTE(pst_startpage+cntr, PTE_STANDARD);
        }

     // now the heap can go right after the PST
     sys_heap->startpage = ADDR_TO_PAGE(_4mb+(cntr*PAGE_SIZE));
     }

  return (pstatus_entry*)_4mb;
}

//////////////////////////////////////////////////////////////////////////////
//  page fault handler for virtual memory manager                           //
//////////////////////////////////////////////////////////////////////////////
TRAP_STUB_ERROR(page_fault)
{
  word pgnum;   // physical page number
  pfparams pfp; // page fault parameters
  word pte0num, pte1num = 0; // numbers of the page table entries
  pt_entry pte;              // page table entry of faulting address

// on a page fault, processor stores faulting address in CR2, get it:
  asm ( " mov %%cr2, %%eax " : "=a" (pfp.addr) );

// get current task pointer
  pfp.curtask = (task *)(((word)taskstate & 0xfffff000)+(PAGE_SIZE-sizeof(task)));
  pfp.state = taskstate;

// enable interrupts so timer, keyboard handler, etc work
//  while handling page fault
  cpu_enable_interrupts();

// get the page directory entry of the faulting address
  pte0num = ADDR_TO_PTL0(pfp.addr);
  pfp.leafpte = &local_pagedir[pte0num];
  pte = *pfp.leafpte;

// if page table present, descend into next level of page tables
  if ( pte & PTE_PRESENT )
     {
     pte1num = ADDR_TO_PTL1(pfp.addr); // get page table entry
     pfp.leafpte = &local_pagetables[pte0num*PTABLE_SIZE+pte1num];
     pte = *pfp.leafpte;

     pgnum = PTE_TO_PAGE(pte);

// Find out if the fault was due to a write to a present, read only page
// (protection violation), or if it was a read/write from a not present page
     if ( taskstate->errcode & PFERR_PROTECTION ) // protection violation
        {
        if ( PGFLT_COPYONWRITE(pte) )
           {
           DEBUG_PAGE_FAULT(PFTYPE_COPYONWRITE, &pfp);
           }
        else if ( taskstate->errcode & PFERR_RESERVED )
           handle_invalidpagefault(PFTYPE_RESERVED, &pfp);
        else handle_invalidpagefault(PFTYPE_WRITE, &pfp);
        }
// The fault was due to a not present page, which could be from
// stack overflow, an allocation, a simple retrieval, or an invalid
// memory access. Check page table entry to find out which.
     else
        {
        // If the page table entry tells us to retrieve the page...
        if ( PGFLT_RETRIEVE(pte) )
           {
           DEBUG_PAGE_FAULT(PFTYPE_RETRIEVE, &pfp);
           // Check page status table of the physical page number that was
           // still left in the pte. If it matches in the PST, simply map it in.
           if ( (pst[pgnum].task == pfp.curtask->tasknum)
                && (pst[pgnum].vpage == ADDR_TO_PAGE(pfp.addr)) )
              {
              vmmsd_removepage(pgnum); // tell swap daemon to remove from buffer
              pst[pgnum].mapped = 1;   // page is mapped again
              *pfp.leafpte |= PTE_PRESENT; // mark as present again
              }
           // Otherwise the page needs to be retrieved from disk. Ask the
           // swap daemon to locate and load the page.
           else
              {
              pgnum = vmmsd_retrievepage(pfp.curtask->tasknum, pgnum);
              pst[pgnum].task = pfp.curtask->tasknum;    // set task number
              pst[pgnum].vpage = ADDR_TO_PAGE(pfp.addr); // set virtual mapping
              pst[pgnum].mapped = 1;    // page is mapped again
              pst[pgnum].pageable = 1;  // page is pageable again
              *pfp.leafpte = MAKE_PTE(pgnum, PTE_STANDARD); // map page
              }
           }
        // If the page table entry tells us to zero allocate a page...
        else if ( PGFLT_ZERO(pte) )
           {
           DEBUG_PAGE_FAULT(PFTYPE_ZERO, &pfp);
           pgnum = vmm_allocateuserpage(PTYPE_USER);  // allocate new page
           pst[pgnum].task = pfp.curtask->tasknum;    // set task number
           pst[pgnum].vpage = ADDR_TO_PAGE(pfp.addr); // set virtual mapping
           pst[pgnum].mapped = 1;
           pst[pgnum].pageable = 1;
           *pfp.leafpte = MAKE_PTE(pgnum, PTE_STANDARD); // set pagetable entry
           vmm_zeropage(ADDR_TO_PAGE(pfp.addr));         // zero the page
           }
        // If the page table entry tells us to allocate a page...
        else if ( PGFLT_ALLOC(pte) )
           {
           DEBUG_PAGE_FAULT(PFTYPE_ALLOC, &pfp);
           pgnum = vmm_allocateuserpage(PTYPE_USER);     // allocate new page
           pst[pgnum].task = pfp.curtask->tasknum;       // set task number
           pst[pgnum].vpage = ADDR_TO_PAGE(pfp.addr);    // set virtual mapping
           pst[pgnum].mapped = 1;                        // mapped
           pst[pgnum].pageable = 1;                      // pageable
           *pfp.leafpte = MAKE_PTE(pgnum, PTE_STANDARD); // set pagetable entry
           }
        else if ( PGFLT_STACK(pte) ) // stack fault, user stack overflow
           {
           DEBUG_PAGE_FAULT(PFTYPE_STACK, &pfp);
           pgnum = vmm_allocateuserpage(PTYPE_STACK);    // allocate new page
           pst[pgnum].task = pfp.curtask->tasknum;       // set task number
           pst[pgnum].vpage = ADDR_TO_PAGE(pfp.addr);    // set virtual mapping
           pst[pgnum].mapped = 1;                        // mapped
           pst[pgnum].pageable = 1;                      // pageable
           *pfp.leafpte = MAKE_PTE(pgnum, PTE_STANDARD); // set pagetable entry
           }
       else if ( PGFLT_NULL(pte) )
          handle_invalidpagefault(PFTYPE_NULL, &pfp);
       else handle_invalidpagefault(PFTYPE_INVALID, &pfp);
       }
    }
// Otherwise the intermediate page table is not present.
// Either it's an invalid address or the pagetable itself needs
// to be located and retrieved.
  else
     {
     }

}

void handle_invalidpagefault(word type, pfparams *pfp)
{
  printstring("\nFATAL ERROR: ", TEXT_ERROR);
  printstring(pfp->curtask->name, TEXT_BRIGHT);
  switch ( type )
     {
     case PFTYPE_WRITE:
        printstring(" attempted a write to a protected portion of memory.", TEXT_NORMAL);
        break;
     case PFTYPE_STACK:
        printstring(" caused a stack overflow error.", TEXT_NORMAL);
        break;
     case PFTYPE_RESERVED:
        printstring(" generated a page table reserved bit error.", TEXT_NORMAL);
        break;
     case PFTYPE_NULL:
        printstring(" attempted a NULL pointer access.", TEXT_NORMAL);
        break;
     case PFTYPE_INVALID:
        printstring(" attempted to access an invalid address.", TEXT_NORMAL);
        break;
     default:
        printstring(" generated an invalid page fault.", TEXT_NORMAL);
     }

  printstring("\nThe instruction at ", TEXT_NORMAL);
  printstring("0x", TEXT_BRIGHT);
  printhex32(pfp->state->eip, TEXT_BRIGHT);
  printstring(" tried to ", TEXT_NORMAL);
  if ( pfp->state->errcode & PFERR_WRITE ) printstring("write ", TEXT_HILIGHT);
  else printstring("read ", TEXT_HILIGHT);
  printstring("address ", TEXT_NORMAL);
  printstring("0x", TEXT_BRIGHT);
  printhex32(pfp->addr, TEXT_BRIGHT);
  printstring(".", TEXT_NORMAL);
  printstring("\n\nThis process will now be terminated.", TEXT_HILIGHT);

  while (1) ;

}


//////////////////////////////////////////////////////////////////////////////
// build_freelist(void)                                                     //
//                                                                          //
//  Initializes the free list for the status table.                         //
//////////////////////////////////////////////////////////////////////////////
void build_freelist(void)
{
  word pgcntr, last = 0;
  word free = 0;

  // initialize the free list
  for ( pgcntr = 0; pgcntr < physpages; pgcntr++ )
     {
     if ( pst[pgcntr].type == PTYPE_FREE ) // find first free page
        {
        listhead[PSTATUS_FREE] = pgcntr;
        last = pgcntr;
        free++;
        break;
        }
     }

  // build the free list
  for ( pgcntr++; pgcntr < physpages; pgcntr++ )
     {
     if ( pst[pgcntr].type == PTYPE_FREE ) // found another free page
        {
        free++;
        pst[last].next = pgcntr; // link last one to this
        pst[pgcntr].next = 0;
        last = pgcntr;              // for next loop, the last is this one
        }
     }

  numpages[PSTATUS_FREE] = free;
}

//////////////////////////////////////////////////////////////////////////////
// vmm_extendheap(heap *hp, word pages)                                     //
//                                                                          //
//  Extend the virtual address space for a larger heap by marking page      //
//  table entries as PGFLT_ALLOC so they are allocated on demand.           //
//////////////////////////////////////////////////////////////////////////////
word vmm_extendheap(heap *hp, word pages)
{
  word cntr;
  word startpage = hp->lastpage,
       endpage = startpage + (pages-1);
  word s_ptl0, s_ptl1, // starting page table entries
       e_ptl0, e_ptl1; // ending page table entries

  DEBUG_PRINT("Heap extension request of ");
  DEBUG_INT(pages);
  DEBUG_STR(" pages, ");

  if ( pages == 0 ) return 0;  // trivial zero allocation
  if ( endpage >= hp->endpage ) // gone beyond the end of the heap
     {
     endpage = hp->endpage - 1;    // stop at end
     pages = endpage - startpage + 1; // update number of pages
     }

  DEBUG_STR("servicing ");
  DEBUG_INT(pages);
  DEBUG_STR(" pages...");
  DEBUG_DELAY(1);

  s_ptl0 = PAGE_TO_PTL0(startpage);
  s_ptl1 = PAGE_TO_PTL1(startpage);
  e_ptl0 = PAGE_TO_PTL0(endpage);
  e_ptl1 = PAGE_TO_PTL1(endpage);

  DEBUG_PRINT("startpage: ");
  DEBUG_H16(startpage);
  DEBUG_STR(", entries: (");
  DEBUG_INT(s_ptl0);
  DEBUG_STR(", ");
  DEBUG_INT(s_ptl1);
  DEBUG_STR(") ");

  DEBUG_PRINT("endpage: ");
  DEBUG_H16(endpage);
  DEBUG_STR(", entries: (");
  DEBUG_INT(e_ptl0);
  DEBUG_STR(", ");
  DEBUG_INT(e_ptl1);
  DEBUG_STR(") ");
  DEBUG_DELAY(1);

  if ( s_ptl0 == e_ptl0 ) // if in the same page table
     {
     // set the range from start to end to PTE_ALLOC in the correct
     // page table.
     vmm_setpagetablerange(s_ptl0, s_ptl1, e_ptl1, PTE_ALLOC);
     }
  else // otherwise, we span two or more page tables
     {
     // |start-pagetable| ... |end-pagetable|
     // set all intervening page tables to be demand allocated
     for ( cntr = s_ptl0+1; cntr < e_ptl0; cntr++ )
        local_pagedir[cntr] = PTE_ALLOC;

     // set the correct ranges in the end page tables to allocate.
     vmm_setpagetablerange(s_ptl0, s_ptl1, PTABLE_SIZE, PTE_ALLOC);
     vmm_setpagetablerange(e_ptl0, 0, e_ptl1, PTE_ALLOC);
     }

 // if this was a system heap change, we need to update the system page
 // directory with the correct top-level page table entries.
  if ( hp == sys_heap )
     {
     for ( cntr = s_ptl0; cntr <= e_ptl0; cntr++ )
        sys_pagedir[cntr] = local_pagedir[cntr];
     }

  hp->lastpage = endpage + 1;

  return pages;
}

//////////////////////////////////////////////////////////////////////////////
// vmm_setpagetablerange(ptnum, start, end, type)                           //
//                                                                          //
//  Allocate or retrieve the page table ptnum for the current address       //
//  space. Set entries "start" up to "end" to value "type."                 //
//////////////////////////////////////////////////////////////////////////////
void vmm_setpagetablerange(word ptnum, word start, word end, pt_entry type)
{
  word pgnum, cntr;

  if ( (local_pagedir[ptnum] & PTE_PRESENT) == 0 ) // not present
     {
     if ( !PGFLT_RETRIEVE(local_pagedir[ptnum]) ) // not a retrieveable
        {                                         // so we must allocate
        pgnum = vmm_allocateuserpage(PTYPE_PTABLE);
        local_pagedir[ptnum] = MAKE_PTE(pgnum, PTE_STANDARD);
        pst[pgnum].vpage = PTENUMS_TO_PAGE(SYS_PDIR, ptnum);
        clear_pagetable(&local_pagetables[ptnum*PTABLE_SIZE]);
        }
     // FYI: if it is a retrieve, we just fall through, take the page fault
     // on the page table and the fault handler will find it for us.
     }

  for ( cntr = start; cntr <= end; cntr++ ) // set the page table entries
     local_pagetables[ptnum*PTABLE_SIZE+cntr] = type;
}

//////////////////////////////////////////////////////////////////////////////
// vmm_allocuserpage(word poolnum, u16i type)                               //
//                                                                          //
//  Allocate a physical page from the user pool specified by poolnum. If    //
//  there are no pages availabe in this pool, return 0.                     //
//////////////////////////////////////////////////////////////////////////////
word vmm_allocateuserpage(u16i type)
{
  word pagenum;

  pagenum = listhead[PSTATUS_FREE];

  if ( pagenum != 0 )  // got a free page
     {
     listhead[PSTATUS_FREE] = pst[pagenum].next;
     numpages[PSTATUS_FREE]--;
     }
  else
     {
     // otherwise, we have to get a page from the page out buffer.
     // that buffer is managed by the swap process which is in
     // memory/swap.c
          pagenum = vmmsd_allocatepage();
     if ( pagenum == 0 ) return 0;
     }

  pst[pagenum].inpob = 0;
  pst[pagenum].next = 0;
  pst[pagenum].pageable = 1; // user page is pageable
  pst[pagenum].mapped = 1;
  pst[pagenum].type = type;

  return pagenum;
}

//////////////////////////////////////////////////////////////////////////////
// vmm_allocsystempage(word poolnum, u16i type)                             //
//                                                                          //
//  Allocate a physical page from the system pool(s).  If there are         //
//  no free pages available, it will ask the swap process for the least     //
//  recently used page. This call is guaranteed to succeed.                 //
//////////////////////////////////////////////////////////////////////////////
word vmm_allocatesystempage(u16i type)
{
  word pagenum;

  pagenum = vmm_allocateuserpage(type); // rely on user mode allocation

  if ( pagenum != 0 )  // got a free page
     pst[pagenum].pageable = 0;

  return pagenum;
}

//////////////////////////////////////////////////////////////////////////////
// vmm_freepage(word pagenum)                                               //
//                                                                          //
//  Free a physical page specified by the page number pagenum.              //
//////////////////////////////////////////////////////////////////////////////
word vmm_freepage(word pagenum)
{
  if ( pagenum >= physpages ) return 0; // invalid page number

  // first we tell swap daemon to remove the page from any page out
  // or dirty buffers it might be in
  vmmsd_removepage(pagenum);

  // then we update the page status table
  pst[pagenum] = default_page;             // this page is now free

  // then insert it back into the free list
  pst[pagenum].next = listhead[PSTATUS_FREE];
  listhead[PSTATUS_FREE] = pagenum;  // it is new head of list
  numpages[PSTATUS_FREE]++;          // increment free pages

  return 1;
}

//////////////////////////////////////////////////////////////////////////////
// clear_pagetable(pagetable pt)                                            //
//                                                                          //
//  Clear a page table by setting all entries to not present and invalid.   //
//////////////////////////////////////////////////////////////////////////////
void clear_pagetable(pagetable pt)
{
  word cntr;

  for ( cntr = 0; cntr < 1024; cntr++ ) // clear the page directory
     pt[cntr] = PTE_INVALID;
}

void vmm_copypage(word pgsrc, word pgdest)
{
  word cntr;
  word *a, *b;

  a = (word *)PAGE_TO_ADDR(pgsrc);
  b = (word *)PAGE_TO_ADDR(pgdest);


  for ( cntr = 0; cntr < PAGE_SIZE/sizeof(word); cntr += 4 )
     {
     b[cntr] = a[cntr];
     b[cntr+1] = a[cntr+1];
     b[cntr+2] = a[cntr+2];
     b[cntr+3] = a[cntr+3];
     }
}

void vmm_zeropage(word pgnum)
{
  word cntr;
  word *a;

  a = (word *)PAGE_TO_ADDR(pgnum);

  for ( cntr = 0; cntr < PAGE_SIZE/sizeof(word); cntr += 4 )
     {
     a[cntr] = 0;
     a[cntr+1] = 0;
     a[cntr+2] = 0;
     a[cntr+3] = 0;
     }
}

void vmm_copyphysicalpage(word pg1, word pg2)
{
}

//////////////////////////////////////////////////////////////////////////////
// enable_paging(pagedir pd)                                                //
//                                                                          //
//  loads CR3 with the page directory base address and enables paging by    //
// setting bit 31 in CR0.                                                   //
//////////////////////////////////////////////////////////////////////////////
void enable_paging(pagetable pd)
{
  asm (
       " mov %0, %%cr3           \n" // load pagedir base
       " mov %%cr0, %%eax        \n" // get CR0
       " or  $0x80000000, %%eax  \n" // set bit 31 (paging)
       " mov %%eax, %%cr0        \n" // write CR0, enable paging
      :
      : "r" (pd) // pd is the input
      );
}

#if ( DEBUG )
void DEBUG_PAGE_FAULT(word type, pfparams *pfp)
{
  word secs = 5;

//  clear();
  printstring("\nPage Fault Encountered in process: ", TEXT_ERROR);
  printstring(pfp->curtask->name, TEXT_HILIGHT);
  printstring("\nInstruction: 0x", TEXT_NORMAL);
  printhex32(pfp->state->eip, TEXT_HILIGHT);
  printstring(", accesing: 0x", TEXT_NORMAL);
  printhex32(pfp->addr, TEXT_HILIGHT);
  printstring("\nFault flags: ", TEXT_NORMAL);

  printstring("\nHandler determined type: ", TEXT_NORMAL);

  switch ( type )
     {
     case PFTYPE_COPYONWRITE:
        printstring("copy on write", TEXT_BRIGHT);
        break;
     case PFTYPE_ALLOC:
        printstring("allocate", TEXT_BRIGHT);
        break;
     case PFTYPE_ZERO:
        printstring("zero allocate", TEXT_BRIGHT);
        break;
     case PFTYPE_RETRIEVE:
        printstring("retrieve", TEXT_BRIGHT);
        break;
     case PFTYPE_STACK:
        printstring("stack allocate", TEXT_BRIGHT);
        break;
     case PFTYPE_RESERVED:
        printstring("reserved bit", TEXT_BRIGHT);
        break;
     case PFTYPE_NULL:
        printstring("null access", TEXT_BRIGHT);
        break;
     case PFTYPE_INVALID:
        printstring("invalid", TEXT_BRIGHT);
        break;
     default:
        printstring("unknown", TEXT_BRIGHT);
     }

  printstring("\n", TEXT_BRIGHT);

  while ( secs > 0 )
     {
     printstring("\rRedpants will handle the fault and will continue in ", TEXT_BRIGHT);
     printlong(secs, TEXT_HILIGHT);
     printstring(" seconds... \b", TEXT_BRIGHT);
     delay(1000);
     secs--;
     }

}
#endif
