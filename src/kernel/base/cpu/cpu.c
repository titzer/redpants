//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-27-2001                                                      //
// Module:  base/cpu/cpu.c                                                  //
// Purpose: This module implements some cpu code that is abstract enough    //
//          to be machine independent.                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/cpu/cpu.h>

#define WAIT_TICKS 1

//////////////////////////////////////////////////////////////////////////////
// measure the clockspeed of the processor                                  //
// note: wraps when cycle difference is greater than 4 billion because 64   //
//       bit divides not built-in in gcc. This limits maximum detectable    //
//       speed depending on CPU_DETECT_TICKS                                //
//////////////////////////////////////////////////////////////////////////////
word cpu_measure_khz(void)
{
  word last, ticks, tr;
  u64i cycles = 0;
  word cyc_diff, khz;

  tr = ktimer_resolution();       // get resolution (ticks per second) of time
  last = ticks = ktimer_ticks(); // get starting ticks

  while ( ticks == last ) ticks = ktimer_ticks(); // wait until start of tick

  last = ticks;           // starting time of loop
  cycles = cpu_cycles();  // starting cycles of loop

  while ( (ticks - last) < WAIT_TICKS ) // wait for x number of ticks
     ticks = ktimer_ticks();

  // read cycles again
  cyc_diff = (word)(cpu_cycles() - cycles)/1000;

  khz = cyc_diff * (tr/WAIT_TICKS); // calculate khz from difference

  return khz;
}

//////////////////////////////////////////////////////////////////////////////
// get our answer closer to "quoted" speed                                  //
// receives parameter in khz, returns answer in mhz                         //
//////////////////////////////////////////////////////////////////////////////
word cpu_estimate_mhz(word khz)
{
  word tolerance;  // khz tolerance for snapping
  word mod33 = khz % 33333; // for check if multiple of 33
  word mod30 = khz % 30000; // for speeds 60, 90, 120 and 180 mhz
  word mod25 = khz % 25000; // for multiples of 25
  word answer = khz; // default is not to alter

  if ( khz < 270000 ) tolerance = 1500; // 1.5 mhz tolerance if less than 270mhz
  else tolerance = khz / 180;           // otherwise, 0.55556% tolerance

  if ( (khz < 200000) && (khz > 50000) ) // test multiples of 30
     {                                   // (p60, p90, p120, ppro180)
     if ( mod30 < tolerance )  answer = (khz/30000)*30000;
     if ( mod30 > (30000 - tolerance) ) answer = ((khz/30000)+1)*30000;
     }

  if ( mod25 < tolerance ) // if just over multiple of 25
     answer = (khz/25000)*25000;
  if ( mod25 > (25000 - tolerance) ) // if just under multiple of 25
     answer = ((khz/25000)+1)*25000;
  if ( mod33 < tolerance ) // if just over multiple of 33
     answer = (khz/33333)*33333;
  if ( mod33 > (33333 - tolerance) ) // if just under multiple of 33
     answer = ((khz/33333)+1)*33333;

  if ( (answer % 1000) > 900 ) // if .9 or above, round up
     answer = (answer/1000)+1;
  else answer = answer/1000;   // turn khz into mhz

  return answer;
}

