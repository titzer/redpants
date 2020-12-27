//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-9-2000                                                        //
// Header:  ia32/devices/pit.h                                              //
// Purpose: This header contains the declarations of the functions that are //
//          available to the kernel for handling timing, including all      //
//          necessary initialization functions.                             //
//          Parts of the timing code have been adapted from the GazOS       //
//          operating system by Gareth Owen, which is also covered under    //
//          the GNU General Public License.                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_PIT_H_
#define _IA32_PIT_H_

#include <base/configure.h>

#define	TMR_CTRL 0x43 /*	I/O for control		*/
#define	TMR_CNT0 0x40 /*	I/O for counter 0	*/
#define	TMR_CNT1 0x41 /*	I/O for counter 1	*/
#define	TMR_CNT2 0x42 /*	I/O for counter 2	*/
#define	TMR_SC0	 0x00 /*	Select channel 0 	*/
#define	TMR_SC1	 0x40 /*	Select channel 1 	*/
#define	TMR_SC2	 0x80 /*	Select channel 2 	*/

#define	TMR_LOW	 0x10 /*	RW low byte only 	*/
#define	TMR_HIGH 0x20 /*	RW high byte only 	*/
#define	TMR_BOTH 0x30 /*	RW both bytes 		*/

#define	TMR_MD0   0x0 /*	Mode 0 			*/
#define	TMR_MD1   0x2 /*	Mode 1 			*/
#define	TMR_MD2   0x4 /*	Mode 2 			*/
#define	TMR_MD3   0x6 /*	Mode 3 			*/
#define	TMR_MD4   0x8 /*	Mode 4 			*/
#define	TMR_MD5   0xA /*	Mode 5 			*/
#define	TMR_BCD   0x1 /*	BCD mode 		*/
#define	TMR_LATCH 0x0 /*	Latch command 		*/
#define	TMR_READ  0xF0 /*    Read command 		*/
#define	TMR_CNT   0x20 /*    CNT bit  (Active low, subtract it) */
#define	TMR_STAT  0x10 /*    Status bit  (Active low, subtract it) */
#define	TMR_CH2   0x8 /*    Channel 2 bit 		*/
#define	TMR_CH1   0x4 /*    Channel 1 bit 		*/
#define	TMR_CH0   0x2 /*    Channel 0 bit 		*/

#define SYS_CHANNEL 0x0
#define TIMER_IRQ 0x0


void pit_init(void);
u64i pit_ticks(void);
word pit_resolution(void);
void pit_delay(u16i ms);

#endif
