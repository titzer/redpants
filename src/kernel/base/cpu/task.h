//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-5-2000                                                        //
// Header:  base/cpu/task.h                                                 //
// Purpose: This module contains the definitions of the structures that are //
//          used by the scheduler to do multitasking.                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_TASK_H_
#define _BASE_TASK_H_

#include <base/configure.h>
#include <base/kernel.h>
#include <base/cpu/cpu.h>

// task states
#define TASK_FREE      0
#define TASK_READY     1
#define TASK_WAITING   2
#define TASK_SLEEPING  3
#define TASK_SUSPENDED 4

#define AFFINITY_FORCE  0x0f // force a process to a specific processor
#define AFFINITY_PREFER 0x08 // process would prefer a certain processor
#define AFFINITY_NOPREF 0x00 // process has no processor preference

#define PRIORITY_REALTIME   0x0f // real time priority
#define PRIORITY_NORMAL     0x09 // normal (application) priority
#define PRIORITY_BACKGROUND 0x03 // background priority
#define PRIORITY_IDLE       0x01 // idle priority
#define PRIORITY_ZOMBIE     0x00 // zombie priority

typedef struct _task
{
  word pid;           // process ID number
  word tasknum;       // task number in kernel task structure

  cstr name;          // process string name

  byte state;         // process state
  byte priority;      // process priority
  byte processor;     // specified processor
  byte affinity;      // affinity for processor

  u64i timeintask;    // time in task
  u64i timeinkernel;  // time in kernel
  u64i waittime;      // time task went into wait state

  cpustate *cstate;   // state of the processor

  void (*entrypoint)(void); // linear address entrypoint

} task;

void  task_init(void);
task *task_create(cstr name, void *entry, word prio);
void  task_exit(void);
void  task_kill(task *);
void  task_suspend(task *);
void  task_resume(task *);
void  task_resched(void);
word  task_curpid(void);
task *task_current(void);
task *task_findbypid(word pid);

#endif
