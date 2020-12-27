//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-25-2001                                                      //
// Header:  base/memory/hat.h                                               //
// Purpose: This module contains the declarations of the structures needed  //
//          for the HAT (hardware address translation) layer of the kernel. //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_HAT_H_
#define _BASE_HAT_H_

typedef struct segment_vtbl {

  word (*create)    (struct segment *);
  word (*destroy)   (struct segment *);

  segment * (*duplicate)(segment *);

  word (*ident)     (struct segment *);
  word (*advise)    (struct segment *, word pgnum, word data);
  word (*fault)     (struct segment *, word type, word pgnum);
  word (*checkop)   (struct segment *, word type, word pgnum);
  word (*flush)     (struct segment *);
  word (*unmaprange)(struct segment *, word start, word end);
  word (*unmappage) (struct segment *, word pgnum);
  word (*lockrange) (struct segment *, word start, word end);
  word (*lockpage)  (struct segment *, word pgnum);

} segment_vtbl;

typedef struct segment {

  segment_vtbl *vtbl;

  word type;
  word start_page;
  word end_page;

} segment;



#endif
