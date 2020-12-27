//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-25-2001                                                      //
// Header:  ia32/cpu/cpu_def.h                                              //
// Purpose: This header makes declarations of the members of the cpu        //
//          structure needed by the IA-32 implementation.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

byte apicid;    /* id of local APIC (used in SMP) */
byte detected;  /* true if processor detected     */
byte family;    /* family number                  */
byte modelnum;  /* model number                   */
byte stepping;  /* stepping number                */
u16i l1d_cache; /* L1 cache data size in KB       */
u16i l1c_cache; /* L1 cache code size in KB       */
u16i l2_cache;  /* L2 cache size in KB            */
u16i mhz;       /* clockspeed in mhz              */
word vendorid;  /* first 32 bits of vendor ID     */
word features;  /* feature flags                  */
word actualkhz; /* actual measured speed in khz   */ 
