//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-25-2001                                                      //
// Header:  base/devices/device.h                                           //
// Purpose: This module contains the declarations of the structures needed  //
//          to implement devices in the redpants kernel.                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_DEVICE_H_
#define _BASE_DEVICE_H_

#include <base/configure.h>
#include <base/util/vmi.h>

//////////////////////////////////////////////////////////////////////////////
// A macro to initiliaze the VTBL entries for a particular device           //
// in the correct order.                                                    //
//////////////////////////////////////////////////////////////////////////////
#define DEVICE_VTBL_ENTRIES(type) \
  VTBL_ENTRY(type,init), \
  VTBL_ENTRY(type,open), \
  VTBL_ENTRY(type,read), \
  VTBL_ENTRY(type,write), \
  VTBL_ENTRY(type,control), \
  VTBL_ENTRY(type,close)

//////////////////////////////////////////////////////////////////////////////
// The device structure.                                                    //
//////////////////////////////////////////////////////////////////////////////
typedef struct device {
  
  ///////////////////////////////////////////////////////////////////////////
  // The device's VTBL.                                                    //
  ///////////////////////////////////////////////////////////////////////////
  struct device_vtbl {

    word (*init)   (struct device *);
    word (*open)   (struct device *);
    word (*read)   (struct device *, void *ptr, word size);
    word (*write)  (struct device *, void *ptr, word size);
    word (*control)(struct device *, word command, word param);
    word (*close)  (struct device *);
    
  } *vtbl;


  word type;
  word number;
  word state;

} device;

#define device_init(this) this->vtbl->init((device*)this)
#define device_open(this) this->vtbl->open((device*)this)
#define device_control(this, c, p) this->vtbl->open((device*)this, c, p)
#define device_read(this, p, s) this->vtbl->open((device*)this, p, s)
#define device_write(this, p, s) this->vtbl->open((device*)this, p, s)
#define device_close(this) this->vtbl->open((device*)this)

  

word devices_init(void);

#endif
