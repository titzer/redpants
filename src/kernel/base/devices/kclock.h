//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-27-2001                                                      //
// Header:  base/devices/kclock.h                                           //
// Purpose: This is the standard kernel clock that keeps track of the time  //
//          of day as well as the date.                                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_KCLOCK_H_
#define _BASE_KCLOCK_H_

#include <base/configure.h>

typedef struct _date
{
  unsigned dayweek   :3; // day of the week
  unsigned daymonth  :5; // day of the month
  byte     month;    // month
  u16i     year;     // year

} date;

typedef struct _time
{
  byte second;  // second
  byte minute;  // minute
  byte hour;    // hour

} time __attribute__ ((packed));

typedef struct _datetime
{
  date ddate;
  time dtime;

} datetime __attribute__ ((packed));

word kclock_init(void);
word kclock_getdate(date *);
word kclock_gettime(time *);
word kclock_setdate(date *);
word kclock_settime(date *);
word kclcok_synctime(time *);

extern cstr months[12];
extern cstr weekdays[7];

#endif
