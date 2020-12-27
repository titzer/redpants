//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    1-01-2001                                                       //
// Header:  base/cpu/locks.h                                                //
// Purpose: This module contains locks for synchronization in SMP and       //
//          mulitprocessing environments. It provides the primitives on     //
//          which more complicated synchronization structures are built.    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_LOCKS_H_
#define _BASE_LOCKS_H_

#include <base/configure.h>

#define LOCKTYPE_SPINLOCK     0
#define LOCKTYPE_SPINLOCK_INT 1
#define LOCKTYPE_READWRITE    2
#define LOCKTYPE_SEMAPHORE    3
#define LOCKTYPE_HYBRID       4

typedef struct lock_vtbl {

  word (*create) (lock *, word param);
  word (*reset)  (lock *);
  word (*try)    (lock *);
  word (*acquire)(lock *);
  word (*release)(lock *);
  word (*destroy)(lock *);

} lock_vtbl;

typedef struct lock {

  lock_vtbl *vtbl;

  word type;
  word state;

  void *ext_state;

} lock;


lock *lock_create(word type, word param);
word lock_try(lock *);
word lock_reset(lock *);
word lock_try(lock *);
word lock_acquire(lock *);
word lock_release(lock *);
word lock_destroy(lock *);

#endif
