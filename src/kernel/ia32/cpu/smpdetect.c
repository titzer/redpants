//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    6-13-2000                                                       //
// Module:  ia32/cpu/smpdetect.c                                            //
// Purpose: This portion of the kernel is responsible for detecting SMP     //
//          support and starting up SMP support on multiple processor       //
//          systems. Based on information from Intel Multiprocessor         //
//          Specification documents.                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <ia32/configure.h>

#define DEBUG_SWITCH DEBUG_SMP_DETECT
#define DEBUG_MODULE "SMP"
#define DEBUG_COLOR  DEBUG_SMP_COLOR

#include <base/kernel.h>
#include <base/util/debug.h>
#include <ia32/cpu/smp.h>

//////////////////////////////////////////////////////////////////////////////
// don't include any of this code if not configured for SMP
#if ( SMP_SUPPORT )
//////////////////////////////////////////////////////////////////////////////

#define MP_SIGNATURE ((u32i)('_')|('M'<<8)|('P'<<16)|('_'<<24))
#define MP_TABLE_SIG ((u32i)('P')|('C'<<8)|('M'<<16)|('P'<<24))
#define BIOS_DATA
#define BIOS_ROM     0xf0000
#define BIOS_ROM_END 0xffff0

#define ADDR(x) ((u8i *)(x))

//////////////////////////////////////////////////////////////////////////////
// local data                                                               //
//////////////////////////////////////////////////////////////////////////////

byte *extended_area;       // extended BIOS data area
mpfloat_struct *mpfloat; // pointer to floating structure
mpconfig_table *mptable; // pointer to configuration table

//////////////////////////////////////////////////////////////////////////////
//  local functions                                                         //
//////////////////////////////////////////////////////////////////////////////

mpfloat_struct *find_mpfloat(byte *addr, u32i size);

void parse_mpfloat(void);
byte cpu_wake(byte apic_id);

//////////////////////////////////////////////////////////////////////////////
//  Detect presence of SMP capable hardware:                                //
//                                                                          //
//  Do this by looking for the floating MP structure which begins with _MP_ //
//  Once we have found it, find the address of the MP config table if it    //
//  exists, and then parse it.
//////////////////////////////////////////////////////////////////////////////
word smp_detect(void)
{
  mpconfig.numcpuents = 0; // start with no cpus detected

  extended_area = (byte *)((*(u16i*)(0x40e)) << 4);  // get start of EBDA

  DEBUG_PRINT("checking EBDA for MP struct...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  mpfloat = find_mpfloat(extended_area, _1kb); // look in first k of EBDA

  if ( mpfloat == NULL )
     {
     DEBUG_PRINT("checking 639-640k for MP struct...");
     DEBUG_DEL(DEBUG_STD_DELAY);
     mpfloat = find_mpfloat(ADDR(0x9ffff), _1kb); // look in last k of conventional
     }
  if ( mpfloat == NULL )
     {
     DEBUG_PRINT("checking BIOS ROM for MP struct...");
     DEBUG_DEL(DEBUG_STD_DELAY);
     mpfloat = find_mpfloat(ADDR(0xf0000), _64kb); // look in BIOS ROM
     }

  if ( mpfloat != NULL )
     {
     DEBUG_PRINT("found MP struct at: 0x");
     DEBUG_H32((u32i)mpfloat);
     DEBUG_DEL(DEBUG_STD_DELAY);
     parse_mpfloat();
     return true;
     }

  DEBUG_PRINT("did not find MP struct...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  return false;
}

//////////////////////////////////////////////////////////////////////////////
//  Initialize SMP support by booting application cpus (APs):         //
//                                                                          //
//  We use their local APIC ids from what we found in the MP table and send //
//  them initialization signals to start them up with some dummy code.      //
//////////////////////////////////////////////////////////////////////////////
word smp_init(void)
{
  byte cntr;
  byte procs = 0;

  DEBUG_PRINT("checking processor entries reported in table...");
  DEBUG_DEL(DEBUG_STD_DELAY);

  for ( cntr = 0; cntr < mpconfig.numcpuents; cntr++ )
     {
     // only init if the processor is not the boot processor
     if ( (mpconfig.cpus[cntr].cpu_flags & BP_BIT) != BP_BIT )
        {
        if ( (mpconfig.cpus[cntr].cpu_flags & EN_BIT) != EN_BIT )
           continue; // BIOS says the processor isn't enabled
        if ( !cpu_wake(mpconfig.cpus[cntr].apic_id) )
           continue; // couldn't initialize this processor
        }

     procs++;
     // everything went ok, insert into system_info structure
     // .. blah blah
     }

  system_info.num_cpus = procs;
  DEBUG_PRINT("Processor initialization completed ");
  DEBUG_DEL(DEBUG_STD_DELAY);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//  Find the MP floating structure in a data area                           //
//////////////////////////////////////////////////////////////////////////////
mpfloat_struct *find_mpfloat(byte *addr, u32i size)
{
  byte found = false;
  u32i cntr = 0;
  mpfloat_struct *mpfl = NULL;

  for ( cntr = 0; cntr < size; cntr++ )    // look through data area
     {
     mpfl = (mpfloat_struct *)addr; // point at address

     if ( mpfl->signature == MP_SIGNATURE) // check signature
        { found = true; break; }

     addr++;                               // look at next address
     }

  if ( found ) return mpfl;

  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
//  Parse the contents of the MP floating structure                         //
//////////////////////////////////////////////////////////////////////////////
void parse_mpfloat(void)
{
  u32i cntr, cpus = 0;
  byte *entry; // entry in config table
  mpcpu_entry temp; // temporary processor entry for reordering

  mpcpu_entry    *proc;
//  mpbus_entry     *bus;
//  mpioapic_entry  *ioapic;
//  mpint_entry     *intent;
//  mpintassn_entry *intassn;

  DEBUG_PRINT("parsing MP floating structure...");
  DEBUG_DEL(DEBUG_STD_DELAY);

  if ( mpfloat->mpconfig != 0 )   // check pointer to mp config table
     {
     DEBUG_PRINT("MP struct has pointer to config: 0x");
     DEBUG_H32(mpfloat->mpconfig);
     DEBUG_DEL(DEBUG_STD_DELAY);
     mptable = (mpconfig_table *)mpfloat->mpconfig; // if nonzero, table exists
     }
  else
     {
     DEBUG_PRINT("MP struct does not have pointer to table");
     DEBUG_DEL(DEBUG_STD_DELAY);
     mptable = NULL;            // no config table
     }

  mpconfig.revision      = mpfloat->revision; // get revision number
  mpconfig.default_conf  = mpfloat->feature1; // get default configuration
  mpconfig.imcr_present  = (mpfloat->feature2 & BIT(7)) == BIT(7);

#if ( DEBUG )
  if ( mpconfig.revision == 1 )
     { DEBUG_PRINT("MP specification version 1.1 reported."); }
  else if ( mpconfig.revision == 4 )
     { DEBUG_PRINT("MP specification version 1.4 reported."); }

  DEBUG_DEL(DEBUG_STD_DELAY);
#endif

  if ( (mptable == NULL) )
     {
     DEBUG_PRINT("No MP table, default configuration: ");
     DEBUG_INT(mpconfig.default_conf);
     DEBUG_DEL(DEBUG_STD_DELAY);
     return; // no table, we're done
     }

  if ( mptable->signature != MP_TABLE_SIG )
     {
     DEBUG_PRINT("MP table does not have correct signature.");
     DEBUG_DEL(DEBUG_STD_DELAY);
     return;
     }

  DEBUG_PRINT("MP table has correct signature.");
  DEBUG_DEL(DEBUG_STD_DELAY);

  DEBUG_PRINT("OEM ID string: ");
// get OEM id string
  for ( cntr = 0; cntr < 8; cntr++ )
     mpconfig.oem_id[cntr] = mptable->oem_id[cntr];
  mpconfig.oem_id[cntr] = 0;
  DEBUG_STR(mpconfig.oem_id);
  DEBUG_DEL(DEBUG_STD_DELAY);

  DEBUG_PRINT("Product ID string: ");
// get product id string
  for ( cntr = 0; cntr < 12; cntr++ )
     mpconfig.prod_id[cntr] = mptable->prod_id[cntr];
  mpconfig.prod_id[cntr] = 0;
  DEBUG_STR(mpconfig.prod_id);
  DEBUG_DEL(DEBUG_STD_DELAY);

  entry = (byte *)mptable;                 // point to start of table
  entry += sizeof(mpconfig_table); // skip to end

// begin parsing entries in table
  for ( cntr = 0; cntr < mptable->entries; cntr++ )
     {
     switch ( *entry )
        {
        case MP_PROC_ENTRY:
           proc = (mpcpu_entry *)entry;  // point to entry
           mpconfig.cpus[cpus] = *proc;   // store contents
           cpus++;                              // increment cpus
           entry += sizeof(mpcpu_entry); // point to next entry
           break;
        case MP_BUS_ENTRY:
           entry += sizeof(mpbus_entry);
           break;
        case MP_IOAPIC_ENTRY:
           entry += sizeof(mpioapic_entry);
           break;
        case MP_INT_ENTRY:
           entry += sizeof(mpint_entry);
           break;
        case MP_INTASSN_ENTRY:
           entry += sizeof(mpintassn_entry);
           break;
        }
     }

  mpconfig.numcpuents = cpus;

// reorder the processor entries so that boot processor is first
  if ( (mpconfig.cpus[0].cpu_flags & BP_BIT) != BP_BIT )
     {
     DEBUG_PRINT("boot strap processor is not first in list, sorting...");
     DEBUG_DEL(DEBUG_STD_DELAY);
     for ( cntr = 1; cntr < cpus; cntr++ ) // look for boot processor
        {
        if ( mpconfig.cpus[cntr].cpu_flags & BP_BIT )
           {
           temp = mpconfig.cpus[0];                      // swap it with
           mpconfig.cpus[0] = mpconfig.cpus[cntr]; // first entry
           mpconfig.cpus[cntr] = temp;
           }
        }
     }

}

//////////////////////////////////////////////////////////////////////////////
//  Initialize a processor by sending inter-processor interrupts to it      //
//  return true if the processor was awakened correctly, false otherwise    //
//////////////////////////////////////////////////////////////////////////////
byte cpu_wake(byte apic_id)
{
  return true; // right now we just assume the processor is ok
}

#endif
