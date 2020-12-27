//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    1-27-2001                                                       //
// Module:  task.h                                                          //
// Purpose: This module contains the intialization code for multitasking    //
//          and some of the important parts of the multitasking unit.       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/configure.h>
#include <base/cpu/task.h>

cstr itaskname = "idle";

task *task_table[MAX_TASKS];

//////////////////////////////////////////////////////////////////////////////
// task_init()                                                              //
//  Initialize the first kernel task structures.                            //
//////////////////////////////////////////////////////////////////////////////
void task_init(void)
{
}

task *task_create(cstr name, void *entry, word priority)
{
  return NULL;
}

void task_suspend(task *t) 
{
}

void task_resume(task *t)
{
}

void task_resched(void)
{
}

task *task_current(void)
{
  return NULL;
}

task *task_findbypid(word pid)
{
  return NULL;
}

