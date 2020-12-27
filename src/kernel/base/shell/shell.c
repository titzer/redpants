//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-13-2000                                                       //
// Module:  base/shell/shell.c                                              //
// Purpose: This portion of the kernel contains a temporary shell that      //
//          allows the user to interact with the system.                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#define _SHELL_ANCHOR_

#include "shell.h"
#include "clib/string.h"
#include <base/kernel.h>
#include <base/devices/kterm.h>
#include <base/devices/keyboard.h>

cstr trimfirstword(cstr s);
cstr trimwhitespace(cstr s);

//////////////////////////////////////////////////////////////////////////////
// startup kernel shell here                                                //
//////////////////////////////////////////////////////////////////////////////
void start_kernel_shell(void)
{
  u8i buffer[256];
  cstr cmdstr;
  cstr params;
  elem *cmd;
  command_stub_type(cmd_hook);

  insert_commands(); // create the basic commands
  shell_buildnum = 52;
  kterm_print("\n", TEXT_NORMAL);
  print_shell_banner();
  kterm_print("For help, use the", TEXT_NORMAL);
  kterm_print(" help", TEXT_HILIGHT);
  kterm_print(" command.\n", TEXT_NORMAL);

  while ( 1 ) // continue with shell indefinately
     {
     kterm_print("\nredpants>", TEXT_NORMAL); // print the prompt

     if ( !getstring(buffer) ) continue;      // if user pressed ESC
     cmdstr = trimwhitespace(buffer);         // clear leading space
     params = trimfirstword(cmdstr);          // otherwise trim off command

     kterm_print("\n", TEXT_NORMAL);          // go to next line

     if ( (cmd = hashtable_find(commands, cmdstr)) == NULL ) // find command
        {
        kterm_print("Unknown command: ", TEXT_ERROR); // not in hashtable
        kterm_print(cmdstr, TEXT_HILIGHT);
        }
     else
        {
        cmd_hook = cmd->data; // get from the hashtable
        if ( cmd_hook != NULL ) cmd_hook(params); // call the command hook
        }
     }
}

void print_shell_banner(void)
{
  kterm_print("Redpants Kernel Shell\n", TEXT_BRIGHT);
  kterm_print("Preliminary %s Build %d", architecture, shell_buildnum);
  kterm_print(", "__DATE__"\n", TEXT_NORMAL);
}

//////////////////////////////////////////////////////////////////////////////
// insert a command into the commands hashtable                             //
//////////////////////////////////////////////////////////////////////////////
void insert_command(cstr s, void *hook)
{
  elem temp;

  temp.str = s;
  temp.data = hook;
  hashtable_insert(commands, &temp);
}

//////////////////////////////////////////////////////////////////////////////
// trim off spaces from beginning and end of string                         //
// return beginning of string                                               //
//////////////////////////////////////////////////////////////////////////////
cstr trimwhitespace(cstr s) // trim whitespace off front and end
{
  cstr k;

  while ( *s == ' ' ) s++; // trim off leading whitespace

  for ( k = s; *k; k++ ) ; // advance to end

  k--;

  while ( k > s ) // loop back to front, cutting off whitespace
     {
     if ( *k != ' ' ) break; // not whitespace, done
     *k = 0;                 // trim off whitespace
     k--;                    // go back another character
     }

  return s; // return beginning of non whitespace
}

//////////////////////////////////////////////////////////////////////////////
// break off first word of string                                           //
// return beginning of next word                                            //
//////////////////////////////////////////////////////////////////////////////
cstr trimfirstword(cstr s)
{
  if ( *s == 0 ) return s; // empty string

  while ( *s != 0 ) // loop until end of string
    {
    if ( *s == ' ' ) // if a space
       { *s = 0; s++; break; } // break off string, return next portion
    s++;
    }

  while ( *s == ' ' ) s++; // skip any more whitespace

  return s; // return beginning of next word
}

