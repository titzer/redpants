//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-22-2000                                                       //
// Module:  cpudetect.c                                                     //
// Purpose: This portion of the kernel is responsible for detecting the     //
//          system processor(s) and their speeds.                           //
//                                                                          //
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Notes: This module can detect the family and type of any Intel, AMD     //
//         or Cyrix processor that supports the CPUID instruction. It       //
//         is capable of detecting the clockspeed of any processor that     //
//         supports the RDTSC instruction. For unknown vendors, it can      //
//         detect the family and model number of the processor and some     //
//         supported features.                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <base/kernel.h>

#define DEBUG_SWITCH DEBUG_CPU_DETECT
#define DEBUG_MODULE "CPU"
#define DEBUG_COLOR  DEBUG_CPU_COLOR

#define CPU_DETECT_ANCHOR

#include <ia32/cpu/cpu.h>
#include <base/cpu/ivm.h>
#include <base/util/debug.h>
#include <base/devices/ktimer.h>

#include <ia32/cpu/smp.h>
#include <ia32/cpu/traps.h>
#include <ia32/cpu/cpuid.h>
#include <ia32/cpu/rdtsc.h>
#include <ia32/devices/pit.h>

// binary representation of instructions
// used by exception handler to ignore invalid opcode exceptions
#define RDTSC_HEX 0x310f

#if ( DEBUG )
 #define DEBUG_FEATURES(x) debug_features(x)
#else
 #define DEBUG_FEATURES(x)
#endif

void detect_cpu(word procnum);
void detect_std_cpu(cpu *, u32i lvl);
void detect_intel_cpu(cpu *, u32i lvl);
void detect_amd_cpu(cpu *, u32i lvl);
void detect_cyrix_cpu(cpu *, u32i lvl);
void debug_features(word features);

//////////////////////////////////////////////////////////////////////////////
// Detect system processors and store results                               //
// This uses the globally available system_info structure                   //
//////////////////////////////////////////////////////////////////////////////
void cpus_detect(void)
{
  u32i cntr;
  cpu defcpu;

  defcpu.id = 0;    // default processor
  defcpu.detected = no;
  defcpu.family = 3;
  defcpu.vendor = CPU_DEFAULT_VENDOR;
  defcpu.model  = CPU_DEFAULT_MODEL;
  defcpu.l1d_cache = CPU_DEFAULT_L1CACHE;
  defcpu.l1c_cache = CPU_DEFAULT_L1CACHE;
  defcpu.l2_cache = CPU_DEFAULT_L2CACHE;
  defcpu.mhz = CPU_DEFAULT_SPEED;
  defcpu.features = 0;

  for ( cntr = 0; cntr < MAX_CPUS; cntr++ )
     {
     system_info.cpus[cntr] = defcpu;
     system_info.cpus[cntr].id = cntr;    // init processor information
     }

 #if ( SMP_SUPPORT )
  DEBUG_PRINT("Entering SMP detection code...");
  DEBUG_DEL(DEBUG_STD_DELAY);

  if ( smp_detect() ) // if we detected presence of SMP
     {
     smp_init(); // configure
     for ( cntr = 0; cntr < system_info.num_cpus; cntr++ )
        detect_cpu(cntr); // probe cpus
     // save processor features
     system_info.cpu_features = system_info.cpus[0].features;
     // store only features that all cpus support
     for ( cntr = 0; cntr < system_info.num_cpus; cntr++ )
        system_info.cpu_features &= system_info.cpus[cntr].features;
     }
  else // otherwise, uniprocessor
 #endif
     {
     system_info.num_cpus = 1;
     detect_cpu(0);  // detect main processor type
     system_info.cpu_features = system_info.cpus[0].features;
     }
}

//////////////////////////////////////////////////////////////////////////////
// detect processor type and mhz                                          //
//////////////////////////////////////////////////////////////////////////////
void detect_cpu(word procnum)
{
  u32i clockspeed = 0;
  u32i highestlvl;          // highest cpuid level supported
  u32i vendor1 = 0,
       vendor2 = 0,
       vendor3 = 0;         // used for cache, vendor stuff
  word cpuidok, rdtscok;

  cpu *thisproc = system_info.cpus + procnum; // current processor

  DEBUG_PRINT("Detecting processor ");
  DEBUG_INT(procnum);
  DEBUG_STR("...");
  DEBUG_DEL(DEBUG_STD_DELAY);

  DEBUG_PRINT("Checking for CPUID instruction...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  cpuidok = cpuid_supported();

  if ( !cpuidok )   // no cpuid supported!
     DEBUG_STR("not supported.");
  else
     {
     thisproc->features |= CPU_FEATURE_CPUID;  // cpuid feature is present
     DEBUG_PRINT("Getting vendor information...");
     DEBUG_DEL(DEBUG_STD_DELAY);
     cpuid(0, &highestlvl, &vendor1, &vendor2, &vendor3); // get vendor
     thisproc->vendorid = vendor1;
     detect_std_cpu(thisproc, highestlvl); // call standard detection routine

     if ( vendor1 == VENDOR_INTEL )
        {
        DEBUG_STR("Intel");
        detect_intel_cpu(thisproc, highestlvl); // detect extra intel features
        }
     else if ( vendor1 == VENDOR_AMD )
        {
        DEBUG_STR("AMD");
        detect_amd_cpu(thisproc, highestlvl);   // detect amd features
        }
     else if ( vendor1 == VENDOR_CYRIX )
        {
        DEBUG_STR("Cyrix");
        detect_cyrix_cpu(thisproc, highestlvl); // detect cyrix features
        }
     }

#if ( CPU_DETECT_SPEED )
  DEBUG_PRINT("Checking for RDTSC instruction...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  rdtscok = rdtsc_supported();               // check to see if RDTSC supported

  if ( rdtscok )                             // RDTSC is supported
     {
     thisproc->features |= CPU_FEATURE_RDTSC;
     DEBUG_PRINT("Detecting processor clockspeed...");
     DEBUG_DEL(DEBUG_STD_DELAY);
     clockspeed = cpu_measure_khz();      // try to measure processor speed
     }
  else
     {
     DEBUG_STR("not supported.");
     }
#endif

  thisproc->actualkhz = clockspeed;                // store measurement
  thisproc->mhz = cpu_estimate_mhz(clockspeed); // clean up answer
  DEBUG_PRINT("Processor khz: ");
  DEBUG_INT(clockspeed);
  DEBUG_STR(", mhz: ");
  DEBUG_INT(thisproc->mhz);
  DEBUG_STR(" ");
  DEBUG_DEL(DEBUG_STD_DELAY);
}

//////////////////////////////////////////////////////////////////////////////
// detect an intel processor                                                //
//////////////////////////////////////////////////////////////////////////////
void detect_intel_cpu(cpu *p, u32i lvl)
{
  DEBUG_STR(" (");
  DEBUG_STR(p->model);
  DEBUG_STR(") reported.");
  DEBUG_FEATURES(p->features);
  DEBUG_DEL(DEBUG_STD_DELAY);
}

//////////////////////////////////////////////////////////////////////////////
// detect an amd processor                                                  //
//////////////////////////////////////////////////////////////////////////////
void detect_amd_cpu(cpu *p, u32i lvl)
{
  u32i eax, ebx, ecx, edx;

  cpuid(0x80000000, &lvl, &eax, &eax, &eax); // check extended level

  if ( lvl >= 1 ) // if supports level 1
     {
     cpuid(0x80000001, &eax, &ebx, &ecx, &edx);  // call extended cpuid

     if ( edx & CPUID_3DNOW  ) p->features |= CPU_FEATURE_3DNOW;
     if ( edx & CPUID_3DNOW2 ) p->features |= CPU_FEATURE_3DNOW2;
     if ( edx & CPUID_X86_64 ) p->features |= CPU_FEATURE_X86_64;
     }

  DEBUG_STR(" (");
  DEBUG_STR(p->model);
  DEBUG_STR(") reported.");
  DEBUG_FEATURES(p->features);
  DEBUG_DEL(DEBUG_STD_DELAY);
}

//////////////////////////////////////////////////////////////////////////////
// detect a cyrix processor                                                 //
//////////////////////////////////////////////////////////////////////////////
void detect_cyrix_cpu(cpu *p, u32i lvl)
{
  u32i eax, ebx, ecx, edx;
 
  // some Cyrix cpus support 3dnow, so test for that
  cpuid(0x80000000, &lvl, &eax, &eax, &eax); // check extended level

  if ( lvl >= 1 ) // if supports level 1
     {
     cpuid(0x80000001, &eax, &ebx, &ecx, &edx);  // call extended cpuid

     if ( edx & CPUID_3DNOW  ) p->features |= CPU_FEATURE_3DNOW;
     if ( edx & CPUID_3DNOW2 ) p->features |= CPU_FEATURE_3DNOW2;
     }

  DEBUG_STR(" (");
  DEBUG_STR(p->model);
  DEBUG_STR(") reported.");
  DEBUG_FEATURES(p->features);
  DEBUG_DEL(DEBUG_STD_DELAY);
}

//////////////////////////////////////////////////////////////////////////////
//  Detect a standard x86 processor                                         //
//  Detects the standard features and family, etc of a processor.           //
//////////////////////////////////////////////////////////////////////////////
void detect_std_cpu(cpu *p, u32i lvl)
{
  u32i eax, ebx, ecx, edx;

  if ( lvl >= 1 ) // if supports family/model detection
    {
    p->detected = true;                          // successfully detected
    cpuid(1, &eax, &ebx, &ecx, &edx);            // call cpuid again
    p->vendor = cpuid_vendor(p->vendorid);
    p->family = CPUID_FAMILY(eax);               // get family
    p->modelnum  = CPUID_MODEL(eax);             // get model
    p->model = cpuid_modelname(p->vendorid,      // get model name
			       p->family,
			       p->modelnum);

  // check feature flags
    if ( edx & CPUID_FPU   ) p->features |= CPU_FEATURE_FPU;
    if ( edx & CPUID_APIC  ) p->features |= CPU_FEATURE_APIC;
    if ( edx & CPUID_PAE   ) p->features |= CPU_FEATURE_PAE;
    if ( edx & CPUID_PGE   ) p->features |= CPU_FEATURE_PGE;
    if ( edx & CPUID_PSE   ) p->features |= CPU_FEATURE_PSE;
    if ( edx & CPUID_PSE36 ) p->features |= CPU_FEATURE_PSE36;
    if ( edx & CPUID_MMX   ) p->features |= CPU_FEATURE_MMX;
    if ( edx & CPUID_SSE   ) p->features |= CPU_FEATURE_SSE;
    if ( edx & CPUID_SSE2  ) p->features |= CPU_FEATURE_SSE2;

    DEBUG_PRINT(p->vendor);
    DEBUG_STR(" Family ");
    DEBUG_INT(p->family);
    DEBUG_STR(", Model ");
    DEBUG_INT(p->modelnum);
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Spill the features for debugging                                        //
//  Test code for processor detection.                                      //
//////////////////////////////////////////////////////////////////////////////
#if ( DEBUG )
void debug_features(u32i features)
{
  DEBUG_PRINT("CPU Features: ");

  if ( features & CPU_FEATURE_CPUID ) DEBUG_STR("CPUID ");
  if ( features & CPU_FEATURE_RDTSC ) DEBUG_STR("RDTSC ");
  if ( features & CPU_FEATURE_FPU )   DEBUG_STR("FPU ");
  if ( features & CPU_FEATURE_APIC )  DEBUG_STR("APIC ");
  if ( features & CPU_FEATURE_PAE )   DEBUG_STR("PAE ");
  if ( features & CPU_FEATURE_PGE )   DEBUG_STR("PGE ");
  if ( features & CPU_FEATURE_PSE )   DEBUG_STR("PSE ");
  if ( features & CPU_FEATURE_PSE36 ) DEBUG_STR("PSE36 ");
  if ( features & CPU_FEATURE_MMX )   DEBUG_STR("MMX ");
  if ( features & CPU_FEATURE_SSE2 )       DEBUG_STR("SSE2 ");
  else if ( features & CPU_FEATURE_SSE )   DEBUG_STR("SSE ");
  if ( features & CPU_FEATURE_3DNOW2 )     DEBUG_STR("3DNOW2 ");
  else if ( features & CPU_FEATURE_3DNOW ) DEBUG_STR("3DNOW ");
  if ( features & CPU_FEATURE_X86_64 )     DEBUG_STR("X86-64 ");
}
#endif

