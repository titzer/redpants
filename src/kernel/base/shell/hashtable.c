//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-12-2000                                                       //
// Module:  hashtable.c                                                     //
// Purpose: This portion of the shell contains the code which implements    //
//          hashtables to allow the console to keep track of commands.      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include "hashtable.h"
#include "clib/string.h"
#include <base/memory/memory.h>

#define DEFAULT_SIZE 31

//hashtable preinit;
//elem pretable[DEFAULT_SIZE];

hashtable *hashtable_create(word size)
{
  word cntr;
  hashtable *h;

// allocate the hashtable structure
  h = (hashtable *)kmm_alloc(sizeof(hashtable));

  if ( h == NULL ) return NULL; // if allocation failed, return NULL

// try to allocate the table
  h->table = (elem *)kmm_alloc(sizeof(elem)*size);

  if ( h->table == NULL ) // if we failed
     {
     kmm_free(h); // free the hashtable structure
     return NULL;    // return NULL
     }

  h->size = size;  // store the size of the table
  h->used = 0;     // none are used yet

  for ( cntr = 0; cntr < size; cntr++ ) // clear all data in hashtable
    {
    h->table[cntr].str = NULL;
    h->table[cntr].data = NULL;
    }

  return h;
}

word hashtable_insert(hashtable *h, elem *e)
{
  word hval;
  word probe;
  elem *f;

  if ( e->str == NULL ) return 0; // don't insert empty strings

  if ( (f = hashtable_find(h, e->str)) != NULL ) // if duplicate
     {
     f->data = e->data;  // update data
     return 1;
     }

  hval = hash(e->str) % h->size;

  if ( h->table[hval].str != NULL ) // if spot is already taken
     {
     for ( probe = 2; probe < 10; probe++ ) // do quadratic probing
        {
        hval = (hval+probe*probe)%h->size;
        if ( h->table[hval].str == NULL ) break;
        }
     if ( probe == 10 ) return 0; // couldn't find a spot after
     }

  h->table[hval] = *e;
  h->used++;

  return 1;
}

word hash(cstr s)
{
 word hash = 0;
 byte c;

  while ( (c = *s++) ) // loop through string
      hash = c + (hash << 6) + (hash << 16) - hash; // update hash value

  return hash;
}

elem *hashtable_find(hashtable *h, cstr s)
{
  word probe = 2;
  word hval;

  if ( s == NULL ) return NULL;

  hval = hash(s) % h->size;

  while ( (h->table[hval].str == NULL) || (strcmp(s, h->table[hval].str) != 0) )
     {
     if ( probe == 9 ) return NULL;     // we didnt find it
     hval = (hval+probe*probe)%h->size; // quadratic probe
     probe++;
     }

  return h->table + hval;
}

void hashtable_list(hashtable *h, void (*callback)(elem *e))
{
  word cntr;

  for ( cntr = 0; cntr < h->size; cntr++ )
     {
     if ( h->table[cntr].str != NULL ) callback(h->table + cntr);
     }
}
