//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-05-2001                                                      //
// Module:  ia32/cpu/locks.c                                                //
// Purpose: This module contains the implementation for locking primitives  //
//          for the IA-32 instruction set.                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/locks.h>


/**
 * Acquire a spinlock. This approach uses conservative locking attempts to
 * eliminate unnecessary bus activity and prevent cache-line ping-ponging
 * in systems of 3 or more processors.
 **/
void spinlock_acquire(spinlock *a)
{
  asm (
       "spinlockwait:     \n"
       " mov (%0), %%eax  \n"
       " jnz spinlockwait \n" /* wait until spinlock is zero before trying */
       "spinlocktry:      \n"
       " lock             \n"
       " bts 0, (%0)      \n"
       " jc spinlockwait  \n" /* unsuccessful attempt: wait till zero again */
       :
       : "p", (a)
       : "%%eax" );
}

void spinlock_release(spinlock *a)
{
  *a = 0;
}

// try to acquire a lock, return true if success, false otherwise
word spinlock_try(spinlock *a)
{
  word result;

  asm (
    " mov $1, %%ebx    \n" // assume we can lock it
    " lock             \n"
    " bts 0, (%0)      \n" // try the lock
    " jnc trylockdone  \n" // not locked, don't write ebx
    " mov $0, %%ebx    \n" // otherwise, we didn't get the lock
    "trylockdone:      \n" 
    : "=b" (result)
    : "p", (a)
    : "%ebx" );

  return result;  
}

