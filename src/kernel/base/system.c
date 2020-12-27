//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-30-2000                                                       //
// Module:  system.c                                                        //
// Purpose: This portion of the kernel contains the major system services   //
//          that are provided to the shell that are less dependant on the   //
//          underlying mechanics of the kernel and more API oriented.       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/kernel.h>
#include <base/system.h>
#include <base/devices/kterm.h>
#include <base/devices/keyboard.h>
#include <base/util/kprint.h>

//////////////////////////////////////////////////////////////////////////////
//  service a system key (CTRL+ALT combination)                             //
//////////////////////////////////////////////////////////////////////////////
void system_service_key(byte key)
{
  switch ( key )
     {
     case SCAN_DEL: // CTRL + ALT + DELETE
        kterm_print("\n %kPlease stand by, rebooting the system... ", TEXT_BRIGHT);
	/** delay(500); **/
        system_reset();
        break;
     }
}

//////////////////////////////////////////////////////////////////////////////
//  service a system event                                                  //
//////////////////////////////////////////////////////////////////////////////
void system_service_event(byte event)
{
  kterm_print("\n %kUnknown system event encountered: %k0x%X",
	      TEXT_ERROR, TEXT_HILIGHT, event);

  kterm_print("\n %kSystem event ignored.", TEXT_NORMAL);
}

//////////////////////////////////////////////////////////////////////////////
//  shutdown kernel                                                         //
//////////////////////////////////////////////////////////////////////////////
byte system_shutdown(void)
{
  kterm_print("Shutting down system...", TEXT_DEFAULT);
  /**  delay(250); **/
  kterm_print("done\n", TEXT_HILIGHT);
  return true;
}

//////////////////////////////////////////////////////////////////////////////
//  hang the system                                                         //
//////////////////////////////////////////////////////////////////////////////
void system_hang(void)
{
  /**
  asm("cli;hlt"); // clear interrupts and halt the processor
  **/
  cpu_disable_interrupts();
  while ( 1 ) ;
}

//////////////////////////////////////////////////////////////////////////////
//  reboot system                                                           //
//////////////////////////////////////////////////////////////////////////////
byte system_reset(void)
{
  /**
   while(inportb(0x64) & 0x02); // try to reboot the computer
   outportb(0x64, 0xfe);
   delay(100);                  // some bioses don't immediately reboot
  **/

   return false;                // obviously we didn't reboot
}


