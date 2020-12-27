//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-27-2001                                                      //
// Header:  ia32/cpu/cpuid.h                                                //
// Purpose: This header file contains declarations of the code and values   //
//          that allow discovery of processor features through the CPUID    //
//          instruction.                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_CPUID_H_
#define _IA32_CPUID_H_

#include <base/cpu/cpu.h>

#define VENDOR_INTEL ((u32i)('G')|('e'<<8)|('n'<<16)|('u'<<24))
#define VENDOR_AMD   ((u32i)('A')|('u'<<8)|('t'<<16)|('h'<<24))
#define VENDOR_CYRIX ((u32i)('C')|('y'<<8)|('r'<<16)|('i'<<24))

#define CPUID_FAMILY(eax) ((eax & 0xf00)>>8)
#define CPUID_MODEL(eax)  ((eax & 0xf0)>>4)
#define CPUID_STEPPING(eax) (eax & 0xf)

#define CPUID_FPU    BIT(0)
#define CPUID_APIC   BIT(9)
#define CPUID_PAE    BIT(6)
#define CPUID_PGE    BIT(13)
#define CPUID_PSE    BIT(3)
#define CPUID_PSE36  BIT(17)
#define CPUID_MMX    BIT(23)
#define CPUID_3DNOW  BIT(31)
#define CPUID_3DNOW2 BIT(30)
#define CPUID_SSE    BIT(25)
#define CPUID_SSE2   BIT(26)
#define CPUID_X86_64 BIT(29)
#define CPUID_IA64   BIT(30)

// Intel processor names, trademarks of Intel Corporation
#define i486sx    "486-SX"
#define i486sx2   "486-SX2"
#define i486dx    "486-DX"
#define i486dx2   "486-DX2"
#define i486sl    "486-SL"
#define i486dx4   "486-DX4"
#define ipent     "Pentium"
#define ipentmmx  "Pentium MMX"
#define ippro     "Pentium Pro"
#define ipent2    "Pentium II"
#define ipent2x   "Pentium II Xeon"
#define ipent3    "Pentium III"
#define ipent3x   "Pentium III Xeon"
#define ipent4    "Pentium 4"
#define icel      "Celeron"
#define ip4f      "486 Family"
#define ip5f      "P5 Family"
#define ip6f      "P6 Family"
#define ip7f      "P7 Family"

#ifdef CPUID_ANCHOR
#define cpuid_global
#else
#define cpuid_global extern
#endif

// Intel processor model table
cpuid_global cstr intel_models[4][16]
#ifdef CPUID_ANCHOR
={
  { i486dx, i486dx, i486sx, i486dx2, i486sl, i486sx2, ip4f, i486dx2,
    i486dx4, ip4f, ip4f, ip4f, ip4f, ip4f, ip4f, ip4f },
  { ip5f, ipent, ipent, ipent, ipentmmx, ipentmmx, ipentmmx, ip5f,
    ip5f, ip5f, ip5f, ip5f, ip5f, ip5f, ip5f, ip5f },
  { ip6f, ippro, ip6f, ipent2, ip6f, ipent2, icel, ipent3,
    ipent3, ip6f, ipent3x, ip6f, ip6f, ip6f, ip6f, ip6f },
  { ipent4, ipent4, ipent4, ip7f, ip7f, ip7f, ip7f, ip7f,
    ip7f, ip7f, ip7f, ip7f, ip7f, ip7f, ip7f, ip7f },
}
#endif // CPUID_ANCHOR
;

// AMD processor names, trademarks of AMD Incorporated, where noted
#define a486dx2  "486-DX2"
#define a486dx4  "486-DX4"
#define a586     "586"
#define ak5      "K5"     /* TM */
#define ak6      "K6"     /* TM */
#define ak62     "K6-2"   /* TM */
#define ak63     "K6-III" /* TM */
#define aath     "Athlon" /* TM */
#define adur     "Duron"  /* TM */

#define ak5f     "K5 Family"
#define ak6f     "K6 Family"
#define ak7f     "K7 Family"

// AMD processor model table
cpuid_global cstr amd_models[3][16]
#ifdef CPUID_ANCHOR
={
  { ak5f, ak5f, ak5f, a486dx2, ak5f, ak5f, ak5f, a486dx2,
    a486dx4, a486dx4, ak5f, ak5f, ak5f, ak5f, a586, a586 },
  { ak5, ak5, ak5, ak5, ak5f, ak5f, ak6, ak6,
    ak62, ak63, ak6f, ak6f, ak6f, ak6f, ak6f, ak6f },
  { ak7f, aath, aath, adur, aath, ak7f, ak7f, ak7f,
    ak7f, ak7f, ak7f, ak7f, ak7f, ak7f, ak7f, ak7f  },
}
#endif // CPUID_ANCHOR
;

// Cyrix processor names, trademarks of Cyrix Corporation, where noted
#define c586   "586"
#define c686   "686"
#define c686f  "686 Family"
#define cm1    "M1-686"     /* TM */
#define cm2    "MII"        /* TM */
#define cm2f   "MII Family"
#define cmgx   "MediaGX"    /* TM */
#define cii    "Cyrix III"  /* TM */

// Cyrix processor model table
cpuid_global cstr cyrix_models[3][16]
#ifdef CPUID_ANCHOR
={
  { c586, c586, c586, c586, c586, c586, c586, c586,
    c586, c586, c586, c586, c586, c586, c586, c586  },
  { c686f, c686f, cm1, c686f, cmgx, c686f, c686f, c686f,
    c686f, c686f, c686f, c686f, c686f, c686f, c686f, c686f  },
  { cm2, cm2f, cm2f, cm2f, cm2f, cii, cm2f, cm2f,
    cm2f, cm2f, cm2f, cm2f, cm2f, cm2f, cm2f, cm2f },
}
#endif // CPUID_ANCHOR
;

word cpuid_supported(void);
void cpuid(u32i lvl, u32i *eax, u32i *ebx, u32i *ecx, u32i *edx);
cstr cpuid_vendor(u32i vendid);
cstr cpuid_modelname(u32i vendid, word family, word model);

#endif
