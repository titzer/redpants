//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    6-13-2000                                                       //
// Header:  ia32/cpu/smp.h                                                  //
// Purpose: This portion of the kernel is responsible for detecting SMP     //
//          support and starting up SMP support on multiple processor       //
//          systems.                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_SMP_H_
#define _IA32_SMP_H_

#include <base/configure.h>
#include <base/kernel.h>
#include <base/cpu/smp.h>

#define DEFAULT_APIC_ADDR 0xFEE00000

#define MP_PROC_ENTRY    0
#define MP_BUS_ENTRY     1
#define MP_IOAPIC_ENTRY  2
#define MP_INT_ENTRY     3
#define MP_INTASSN_ENTRY 4

#define BP_BIT BIT(1)
#define EN_BIT BIT(0)

// the structure defined by the BIOS to describe MP configurations
// for more information see Intel Multiprocessing Specification
typedef struct mpfloat_struct_
{
  u32i signature;  // signature
  u32i mpconfig;   // address of config table
  u8i  length;     // length in 16 byte units
  u8i  revision;   // revision number
  u8i  checksum;   // checksum of bytes
  u8i  feature1;   // feature byte 1
  u8i  feature2;   // feature byte 2
  u8i  feature3;   // feature byte 3
  u8i  feature4;   // feature byte 4
  u8i  feature5;   // feature byte 5

} mpfloat_struct __attribute__ ((packed));

// configuration table header
// for more information see Intel Multiprocessing Specification
typedef struct mpconfig_table_
{
  u32i signature;    // signature
  u16i base_length;  // length of base table
  u8i  revision;     // revision
  u8i  checksum;     // checksum of bytes
  u8i  oem_id[8];    // oem identifier
  u8i  prod_id[12];  // product identifier
  u32i oem_table;    // pointer to OEM table
  u16i oem_size;     // size of OEM table
  u16i entries;      // number of entries
  u32i apic_addr;    // address of local APIC
  u16i ext_length;   // extended table length
  u8i  ext_checksum; // checksum of extended table
  u8i  reserved;     // reserved space

} mpconfig_table __attribute__ ((packed));

// processor entry in MP configuration table
typedef struct mpcpu_entry_
{
  u8i  entry_type;  // 0 for processor entry
  u8i  apic_id;     // id of the local APIC
  u8i  apic_ver;    // version of the local APIC
  u8i  cpu_flags;   // boot processor and enabled flags
  u32i signature;   // processor signature (CPUID)
  u32i features;    // feature flags
  u32i reserved1;   // reserved space
  u32i reserved2;   // reserved space

} mpcpu_entry __attribute__ ((packed));

// bus entry in MP configuration table
typedef struct mpbus_entry_
{
  u8i entry_type;    // 1 for bus entry
  u8i bus_id;        // id of bus
  u8i idstring[6];   // string of bus type

} mpbus_entry __attribute__ ((packed));

// IO APIC entry in MP configuration table
typedef struct mpioapic_entry_
{
  u8i  entry_type;
  u8i  apic_id;
  u8i  apic_ver;
  u8i  apic_flags;
  u32i apic_addr;

} mpioapic_entry __attribute__ ((packed));

typedef struct mpint_entry_
{
  u32i unused1;
  u32i unused2;

} mpint_entry __attribute__ ((packed));

typedef struct mpintassn_entry_
{
  u32i unused1;
  u32i unused2;

} mpintassn_entry __attribute__ ((packed));

typedef struct apic_interface_
{
  u32i unknown;  // interface of APIC not known yet

} apic_interface __attribute__ ((packed));

typedef struct mpconfig_struct_
{
  u8i oem_id[9];    // null terminated OEM id
  u8i prod_id[13];  // null terminated product id
  u8i revision;     // revision number
  u8i default_conf; // default configuration
  u8i imcr_present; // true of IMCR present
  u8i numcpuents;  // number of processor entries

  mpcpu_entry cpus[SMP_MAX_CPUS];
} mpconfig_struct __attribute__ ((packed));

global apic_interface *localapic
  globalinit((apic_interface*)DEFAULT_APIC_ADDR);

global mpconfig_struct mpconfig;

// accessable functions
word smp_detect(void);

#endif
