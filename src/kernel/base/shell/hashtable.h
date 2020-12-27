//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-12-2000                                                       //
// Header:  base/shell/hashtable.h                                          //
// Purpose: These are the declarations for the hashtables that the shell    //
//          uses to manage commands.                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _SHELL_HASHTABLE_H_
#define _SHELL_HASHTABLE_H_

#include <base/configure.h>

typedef struct _elem
{
  cstr str;    // string key
  void *data;  // the data
} elem;

typedef struct _hashtable
{
  word size, used;
  elem *table;
} hashtable;

hashtable *hashtable_create(word size);

word  hashtable_insert(hashtable *, elem *);
void  hashtable_resize(hashtable *);
elem *hashtable_find  (hashtable *, cstr);
void  hashtable_list  (hashtable *, void (*callback)(elem *e));

word hash(cstr); // hash function

#endif
