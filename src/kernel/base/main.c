//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-12-2000                                                       //
// Module:  main.c                                                          //
// Purpose: This is the kernel, the main portion of the operating system,   //
//          that handles all the duties of a modern operating system.       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      The Redpants Operating System is free software. You can freely      //
//  modify and distribute this software under the terms of the GNU General  //
//  Public License available at:                                            //
//                                                                          //
//        http://www.gnu.org/copyleft/gpl.html                              //
//                                                                          //
//      This is preliminary build software. The authors make no guarantee   //
//  of the fitness of this software for any particular purpose and cannot   //
//  be held responsible for any damages done to your computer system(s),    //
//  whether hardware or software.                                           //
//                                                                          //
//      If you make any modifications to this software the extend its       //
//  capabilities or make corrections that fix problems in this software,    //
//  you are encouraged to share your contribution with the original         //
//  authors and make your modifications public by distributing them.        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  You can visit the official redpants development website at:             //
//                                                                          //
//        www.redpants.org                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define _KERNEL_ANCHOR_

#include <base/kernel.h>
#include <base/system.h>

#include <base/cpu/ivm.h>
#include <base/cpu/cpu.h>
#include <base/cpu/smp.h>
#include <base/cpu/task.h>

#include <base/devices/device.h>
#include <base/devices/kterm.h>

#include <base/memory/memory.h>
#include <base/memory/vmm.h>

#include <base/shell/shell.h>

void printbuild(word buildnum);
void printsysteminfo(void);
void start_shell(void);
void printbanner(void);

//////////////////////////////////////////////////////////////////////////////
//  entrypoint of the kernel                                                //
//////////////////////////////////////////////////////////////////////////////

#include <target/entrypoint.h>

//////////////////////////////////////////////////////////////////////////////
//  main function of the kernel                                             //
//////////////////////////////////////////////////////////////////////////////
int main(void)
{
  ivm_init();       // initialize interrupt vector manager
  kterm_init();     // initialize kernel terminal
  kterm_clear();    // clear the screen
  printbanner();    // print Redpants banner

  devices_init();   // initialize built in devices
  cpu_detect();     // detect CPUs
  smp_init();       // initialize smp
  kmm_init();       // initialize kernel memory manager
  vmm_init();
  //  pit_init();      // start up periodic timer (PIT)
  //  rtc_init();      // initialize real time clock


  printsysteminfo();       // print what we have detected
  start_shell(); // enter shell

  while ( 1 ) ;

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//  printbanner: print the banner in lower right                            //
//////////////////////////////////////////////////////////////////////////////
void printbanner(void)
{
  kterm_print("Redpants Operating System Kernel\n");
  kterm_print(BUILD_STR "\n");
}


//////////////////////////////////////////////////////////////////////////////
//  print processor/memory information                                      //
//////////////////////////////////////////////////////////////////////////////
void printsysteminfo(void)
{
  word gigram;
  word mhz, ghz;

  if ( system_info.num_cpus > 1 )
    kterm_print("(%d)",system_info.num_cpus);

  kterm_print(" %s %s", system_info.cpus[0].vendor, system_info.cpus[0].model);

  if ( (mhz = system_info.cpus[0].mhz) > 0 )
     {
     if ( mhz >= 1000 ) // report in ghz
        {
        ghz = mhz/1000;
        mhz = mhz%1000;
	kprint(" %d", ghz);
        ghz = mhz/100;
        mhz = mhz%100;
        if ( ghz > 0 )
           {
	   kprint(".%d", ghz);
           ghz = mhz/10;
           mhz = mhz%10;
           if ( ghz > 0 )
              kterm_print("%d", ghz);
           }
        kterm_print("ghz\n");
        }
     else kterm_print(" %d mhz", mhz);
     }

  //  movecurs_box(&headerline, 48, 1);
  kterm_print("System Memory: ");

  if ( ((system_info.physical_memory % 1024) == 0) ||
        (system_info.physical_memory > 4096) )
     {
     gigram = system_info.physical_memory >> 10; // report in gigs
     kterm_print("%d gigabyte", gigram);
     if ( gigram > 1 ) kterm_print("s");
     }
  else
     {
     kterm_print("%d megabytes", system_info.physical_memory);
     }

  kterm_print(" system memory");
}

//////////////////////////////////////////////////////////////////////////////
//  startup the shell, whether internally or externally                     //
//////////////////////////////////////////////////////////////////////////////
void start_shell(void)
{
 #if ( KERNEL_SHELL )
   // startup the built in kernel shell
   // this source is in redpants/development/kernel/shell
   start_kernel_shell();
 #else
   // use an external shell, but we don't have one yet
   // so we just print a message and die
   kterm_print("No external shell found.\n");
 #endif
}

