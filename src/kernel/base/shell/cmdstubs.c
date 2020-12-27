//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-17-2000                                                       //
// Module:  base/shell/cmdstubs.c                                           //
// Purpose: This portion of the shell contains the code stubs for most of   //
//          the shell's built in commands.                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/kernel.h>
#include <base/system.h>

#include <base/shell/shell.h>
#include <base/shell/clib/string.h>

#include <base/memory/memory.h>
#include <base/memory/vmm.h>

#include <base/devices/kterm.h>
#include <base/devices/ktimer.h>
#include <base/devices/kclock.h>
#include <base/devices/keyboard.h>

#include <base/shell/tetris.h>

#define COMMAND_STUB(x) void cmd_##x(cstr args)
#define INSERT_COMMAND(x) insert_command(#x, cmd_##x);
#define SCROLLBENCH_SIZE ((u32i)5000)
#define PUTSBENCH_ITERS ((u32i)15000)
#define COPYBENCH_ITERS ((u32i)400)
#define COPYBENCH_PAGES 256
#define PUTSBENCH_LINES ((u32i)19)

COMMAND_STUB(reboot);
COMMAND_STUB(uptime);
COMMAND_STUB(system_shutdown);
COMMAND_STUB(kterm_clear);
COMMAND_STUB(echo);
COMMAND_STUB(help);
COMMAND_STUB(list);
COMMAND_STUB(heapstatus);
COMMAND_STUB(time);
COMMAND_STUB(date);
COMMAND_STUB(tetris);
COMMAND_STUB(printbench);
COMMAND_STUB(scrollbench);
COMMAND_STUB(copybench);
COMMAND_STUB(pftest);

byte confirm(cstr, byte, byte);
void list_callback(elem *e);

//////////////////////////////////////////////////////////////////////////////
// setup the basic commands                                                 //
//////////////////////////////////////////////////////////////////////////////
void insert_commands(void)
{
  static byte inserted = false;

  if ( inserted ) return; // don't insert more than once

  commands = hashtable_create(31); // create the commands hashtable

  if ( commands == NULL )
     {
     kterm_print("ERROR: Could not create shell hashtable\n", TEXT_ERROR);
     return;
     }

  INSERT_COMMAND(reboot);
  INSERT_COMMAND(uptime);
  INSERT_COMMAND(system_shutdown);
  INSERT_COMMAND(kterm_clear);
  INSERT_COMMAND(echo);
  INSERT_COMMAND(help);
  INSERT_COMMAND(list);
  INSERT_COMMAND(heapstatus);
  INSERT_COMMAND(time);
  INSERT_COMMAND(date);
  INSERT_COMMAND(tetris);
  INSERT_COMMAND(printbench);
  INSERT_COMMAND(copybench);
  INSERT_COMMAND(scrollbench);
  INSERT_COMMAND(pftest);

  inserted = true;
}

//////////////////////////////////////////////////////////////////////////////
// Command: reboot                                                          //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(reboot)
{
  if ( confirm("Are you sure you want to reboot?", no, TEXT_HILIGHT) )
     {
     kterm_print("Please standy while rebooting the system...", TEXT_BRIGHT);
     ktimer_delay(250); // wait a quarter of a second
     system_reset();   // try to reboot
     }
}

//////////////////////////////////////////////////////////////////////////////
// Command: uptime                                                          //
//  Action: display system uptime                                           //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(uptime)
{
  word seconds = 0, minutes, hours, tr;

  tr = ktimer_resolution();                // get timer resolution
  seconds = ((u32i)ktimer_ticks()) / tr; // get elapsed seconds

  kterm_print("System uptime: ", TEXT_NORMAL);
  hours = seconds / 3600;                 // calculate hours
  minutes = (seconds / 60) - (hours*60);  // minutes
  seconds = seconds % 60;                 // seconds

  if ( hours )                            // if any hours, print them
     {
     kterm_print("%d",hours, TEXT_HILIGHT);
     kterm_print(":", TEXT_HILIGHT);
     if ( minutes < 10 )                // if minutes value is 1 digit
        kterm_print("0", TEXT_HILIGHT); // print leading zero
     }

  kterm_print("%d",minutes, TEXT_HILIGHT);     // print minutes
  kterm_print(":", TEXT_HILIGHT);

  if ( seconds < 10 )                 // if seconds is one digit
     kterm_print("0", TEXT_HILIGHT);  // print leading zero

  kterm_print("%d",seconds, TEXT_HILIGHT);
}

//////////////////////////////////////////////////////////////////////////////
// Command: system_shutdown                                                        //
//  Action: system_shutdown system (asks for confirmation from user)               //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(system_shutdown)
{
  if ( confirm("Are you sure you want to system_shutdown the system?", no, TEXT_HILIGHT) )
     {
     system_shutdown();
     kterm_print("It is now safe to power down the system.", TEXT_NORMAL);
     asm("cli;hlt");
     }
}

//////////////////////////////////////////////////////////////////////////////
// Command: kterm_clear                                                           //
//  Action: kterm_clear current console                                           //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(kterm_clear)
{
  kterm_clear();
}

//////////////////////////////////////////////////////////////////////////////
// Command: tetris                                                          //
//  Action: runs tetris program                                             //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(tetris)
{
  run_tetris();
}

//////////////////////////////////////////////////////////////////////////////
// Command: echo <str>                                                      //
//  Action: echoes the specified string to the console                      //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(echo)
{
  kterm_print(args, TEXT_NORMAL);
}

//////////////////////////////////////////////////////////////////////////////
// Command: help <cmd>                                                      //
//  Action: finds help for the specified command                            //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(help)
{
  if ( strlen(args) == 0 )
     {
     print_shell_banner();
     kterm_print("\n For a list of commands, use the command ", TEXT_NORMAL);
     kterm_print("list commands", TEXT_HILIGHT);
     kterm_print(".\n For more help on a specific command, use the ", TEXT_NORMAL);
     kterm_print("help ", TEXT_HILIGHT);
     kterm_print("command\n followed by the name of the command.\n", TEXT_NORMAL);
     }
  else
     {
     kterm_print("No help available for command: ", TEXT_ERROR);
     kterm_print(args, TEXT_HILIGHT);
     }
}

COMMAND_STUB(list)
{
  if ( strcmp(args, "commands") == 0 )
     {
     hashtable_list(commands, list_callback);
     }
  else
     {
     kterm_print("Not able to list specified item(s): ", TEXT_ERROR);
     kterm_print(args, TEXT_HILIGHT);
     }

}

COMMAND_STUB(heapstatus)
{
  hblk_head *mb;
  fblk_head **fl;
  word size, totalsize = 0, totalfree = 0;
  byte alloc;

  mb = kmm_getfirstblock();
  fl = kmm_getfreelists();

  kterm_print("Heap status shell/kernel debugging feature\n", TEXT_BRIGHT);
  kterm_print("This command dumps the state of the kernel space heap.\n\n", TEXT_NORMAL);

  kterm_print("Block status:\n", TEXT_NORMAL);
  kterm_print("Start", TEXT_NORMAL);
  kterm_movecol(15); // move to column
  kterm_print("Size", TEXT_NORMAL);
  kterm_movecol(26); // move to column
  kterm_print("Status", TEXT_NORMAL);
  kterm_movecol(40); // move to column
  kterm_print("Flags\n", TEXT_NORMAL);

  while ( mb != NULL )
    {
    size = mb->size & ~(BIT(0));

    if ( mb->size & BIT(0) ) // if used
       alloc = 1;
    else
       {
       alloc = 0;
       totalfree += size;
       }

    totalsize += size;

    kterm_print(" %x",(word)mb);
    kterm_movecol(16); // move to column
    kterm_print("%d",size, TEXT_HILIGHT);
    kterm_movecol(27); // move to column
    if ( alloc ) kterm_print("allocated", LIGHTCYAN);
    else kterm_print("free", CYAN);
    kterm_movecol(41); // move to column
    kterm_print("locked", LIGHTRED);
    kterm_print("\n", TEXT_NORMAL);

    mb = mb->next;
    }

  kterm_print("\nTotal heap size: ", TEXT_NORMAL);
  kterm_print("%d",totalsize, TEXT_HILIGHT);
  kterm_print(" bytes", TEXT_HILIGHT);
  kterm_print(", free: ", TEXT_NORMAL);
  kterm_print("%d",totalfree, TEXT_HILIGHT);
  kterm_print(" bytes\n", TEXT_HILIGHT);

//  kterm_print("Press any key to continue...", TEXT_NORMAL);
//  if ( getch() == 0 ) getch();
}

COMMAND_STUB(date)
{
  date d;

  kclock_getdate(&d);

  kterm_print(weekdays[d.dayweek], TEXT_HILIGHT);
  kterm_print(", ", TEXT_HILIGHT);
  kterm_print(months[d.month-1], TEXT_HILIGHT);
  kterm_print(" ", TEXT_HILIGHT);
  kterm_print("%d",d.daymonth, TEXT_HILIGHT);
  kterm_print(", ", TEXT_HILIGHT);
  kterm_print("%d",d.year, TEXT_HILIGHT);
}

COMMAND_STUB(time)
{
  time t;
  u32i pm = 0;

  kclock_gettime(&t);

  if ( t.hour > 12 )
     { pm = 1; t.hour -= 12; }
  kterm_print("%d",t.hour, TEXT_HILIGHT);
  kterm_print(":", TEXT_HILIGHT);
  if ( t.minute < 10 ) kterm_putch('0');
  kterm_print("%d",t.minute, TEXT_HILIGHT);
  kterm_print(":", TEXT_HILIGHT);
  if ( t.second < 10 ) kterm_putch('0');
  kterm_print("%d",t.second, TEXT_HILIGHT);

  if ( pm ) kterm_print(" PM", TEXT_HILIGHT);
  else kterm_print(" AM", TEXT_HILIGHT);
}

void list_callback(elem *e)
{
  kterm_print(e->str, TEXT_HILIGHT);
  kterm_print("\n", TEXT_NORMAL);
}


//////////////////////////////////////////////////////////////////////////////
// benchmarks how fast the console code can scroll                          //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(scrollbench)
{
  word cntr;
  word ticks = ktimer_ticks();
  word fract;
  //  word colors[4] = { BLUE<<4, GREEN<<4, RED<<4, BLACK<<4 };
  //  byte oldattr;
  //  textbox *tb = (textbox *)get_default_box();

  //  oldattr = tb->attr;

  for ( cntr = 0; cntr < SCROLLBENCH_SIZE; cntr++)
     {
       //     tb->attr = colors[(cntr>>2)&0x03];
     kterm_print("\n", TEXT_DEFAULT);
     }

  //  tb->attr = oldattr;
  ticks = ktimer_ticks() - ticks;

  ticks *= 1000;
  ticks /= ktimer_resolution(); // ticks now is milliseconds

  kterm_print("\nTotal time: ", TEXT_BRIGHT);
  kterm_print("%d",ticks/1000, TEXT_HILIGHT);
  kterm_print(".", TEXT_HILIGHT);
  fract = ticks % 1000;
  if ( fract < 10 ) kterm_print("00", TEXT_HILIGHT);
  else if ( fract < 100 ) kterm_print("0", TEXT_HILIGHT);
  kterm_print("%d",fract, TEXT_HILIGHT);
  kterm_print(" secs", TEXT_HILIGHT);
  kterm_print("\nLines per second: ", TEXT_BRIGHT);
  kterm_print("%d",SCROLLBENCH_SIZE*1000/ticks, TEXT_HILIGHT);
}

//////////////////////////////////////////////////////////////////////////////
// benchmarks how fast the console code can print                           //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(printbench)
{
  word cntr;
  word ticks;
  word fract;
  word lines, chars;
  cstr pstrd = "Benchmarking printing functions";
  cstr pstr;
  byte colors[6] = { LIGHTMAGENTA, LIGHTRED, LIGHTGREEN,
                     LIGHTBLUE, YELLOW, WHITE };

  pstr = kmm_alloc(PUTSBENCH_LINES*80);

  for ( cntr = 0; pstrd[cntr]; cntr++ )
     pstr[cntr] = pstrd[cntr];

  for ( ; cntr < PUTSBENCH_LINES*80-1; cntr++ )
     pstr[cntr] = '.';

  pstr[cntr] = 0;

  kterm_clear();
  ticks = ktimer_ticks();

  for ( cntr = 0; cntr < PUTSBENCH_ITERS; cntr++)
     {
     kterm_movecursor(0, 0);
     kterm_print(pstr, colors[(cntr>>10)%6]);
     }

  ticks = ktimer_ticks() - ticks;

  ticks *= 1000;
  ticks /= ktimer_resolution(); // ticks now is milliseconds

  kterm_print("\nTotal time: ", TEXT_BRIGHT);
  kterm_print("%d",ticks/1000, TEXT_HILIGHT);
  kterm_print(".", TEXT_HILIGHT);
  fract = ticks % 1000;
  if ( fract < 10 ) kterm_print("00", TEXT_HILIGHT);
  else if ( fract < 100 ) kterm_print("0", TEXT_HILIGHT);
  kterm_print("%d",fract, TEXT_HILIGHT);
  kterm_print(" secs", TEXT_HILIGHT);
  kterm_print("\nLines per second: ", TEXT_BRIGHT);
  lines = (PUTSBENCH_ITERS*PUTSBENCH_LINES*1000)/ticks;
  kterm_print("%d",lines, TEXT_HILIGHT);
  chars = lines*80;
  kterm_print("\nCharacters per second: ", TEXT_BRIGHT);
  kterm_print("%d",chars, TEXT_HILIGHT);

  kmm_free(pstr);
}

//////////////////////////////////////////////////////////////////////////////
// benchmarks how fast the vmm can copy a page of memory                    //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(copybench)
{
  word cntr;
  word page;
  word ticks;
  word fract;
  word pages, kbytes;

  kterm_print("Benchmarking vmm_copypage() call...", TEXT_BRIGHT);

  ticks = ktimer_ticks();

  for ( cntr = 0; cntr < COPYBENCH_ITERS; cntr++)
     {
     for ( page = 0x2f0; page > (0x2f0-COPYBENCH_PAGES); page-- )
        vmm_copypage(page, page-1);
     }

  ticks = ktimer_ticks() - ticks;

  ticks *= 1000;
  ticks /= ktimer_resolution(); // ticks now is milliseconds

  kterm_print("\nTotal time: ", TEXT_BRIGHT);
  kterm_print("%d",ticks/1000, TEXT_HILIGHT);
  kterm_print(".", TEXT_HILIGHT);
  fract = ticks % 1000;
  if ( fract < 10 ) kterm_print("00", TEXT_HILIGHT);
  else if ( fract < 100 ) kterm_print("0", TEXT_HILIGHT);
  kterm_print("%d",fract, TEXT_HILIGHT);
  kterm_print(" secs", TEXT_HILIGHT);
  kterm_print("\nPages per second: ", TEXT_BRIGHT);
  pages = (COPYBENCH_PAGES*COPYBENCH_ITERS*1000)/ticks;
  kterm_print("%d",pages, TEXT_HILIGHT);
  kbytes = pages*(PAGE_SIZE/1024);
  kterm_print("\nKB per second: ", TEXT_BRIGHT);
  kterm_print("%d",kbytes, TEXT_HILIGHT);
}

//////////////////////////////////////////////////////////////////////////////
// benchmarks how fast the console code can print                           //
//////////////////////////////////////////////////////////////////////////////
COMMAND_STUB(pftest)
{
  word cntr;

  cstr msg = "This is a Redpants internal debugging command. ",
    msg2 = "It tests the VMM's paging\n"
    "exception handling. Since this test is run in kernel mode, it is normal\n"
    "for the system to encounter an unrecoverable error.\n\n";

  
  kterm_print(msg, TEXT_BRIGHT);
  kterm_print(msg2, TEXT_NORMAL);
  
  if ( !confirm("Do you want to continue?", false, TEXT_BRIGHT) ) return;


  for ( cntr = 0; cntr < 0xFFF00000; cntr += 0x100000 )
     {
     kterm_print("\nTesting pages: ", TEXT_NORMAL);
     kterm_print("%d",cntr/1048576, TEXT_HILIGHT);
     kterm_print("-", TEXT_HILIGHT);
     kterm_print("%d",(cntr/1048576)+1, TEXT_HILIGHT);
     kterm_print("mb", TEXT_HILIGHT);


     asm volatile
      (
        " mov %%eax, %%ebx          \n"
        " add $1048576, %%eax       \n"
        "looppftest:                \n"
        " mov (%%ebx), %%ecx        \n"
        " add $4096, %%ebx          \n"
        " cmp %%eax, %%ebx          \n"
        " jle looppftest            \n"

        : : "a" (cntr) : "%ebx", "%ecx"
        );
     }


//  asm ( "mov $0, %ecx ; div %ecx " );

}

//////////////////////////////////////////////////////////////////////////////
// displays message and accepts only y or n                                 //
//////////////////////////////////////////////////////////////////////////////
byte confirm(cstr prompt, u8i cur, u8i attr)
{
  byte ch;

  kterm_print(prompt, attr);       // print the prompt
  kterm_print(" (y, n):  ", attr);

  while ( 1 ) // loop forever
    {
    if ( cur ) kterm_print("\by", TEXT_NORMAL); // if currently yes, print y
    else kterm_print("\bn", TEXT_NORMAL);       // otherwise print n

    ch = getch(); // get a character from keyboard

    if ( ch == 0 ) { ch = getch(); continue; } // if 0, ignore extended code
    if ( (ch == 'y') || (ch == 'Y') ) cur = yes; // update to yes
    if ( (ch == 'n') || (ch == 'N') ) cur = no;  // update to no

    if ( ch == 13 ) break; // enter, accept
    }

  kterm_print("\n", attr); // go to next line
  return cur;              // return
}
