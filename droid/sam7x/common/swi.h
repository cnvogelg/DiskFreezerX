/* done by Martin Thomas */

/* This needs more testing... */

#ifndef SWI_H_
#define SWI_H_

#include "swi_numbers.h"

static inline unsigned long do_SWI( const int swi_num, 
	unsigned long par0,
	unsigned long par1,
	unsigned long par2,
	unsigned long par3 )
{
	unsigned long ret_val;

	asm volatile (
		"mov r0, %2 \n\t" \
		"mov r1, %3 \n\t" \
		"mov r2, %4 \n\t" \
		"mov r3, %5 \n\t" \
		"swi %a1    \n\t" \
		"mov %0, r0 \n\t" \
		:	"=r" (ret_val)
		:	"I" ((const int)swi_num), 
			"r" (par0), "r" (par1), "r" (par2), "r" (par3)
		:	"r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
	);

	return ret_val;
}

#define SWI_CALL(MY_SWI_ID) \
	asm volatile( \
	"swi %a0     \n\t" \
	: : "I" (MY_SWI_ID) : "lr")

#define MY_SWI_CALL_RES(MY_SWI_ID, MY_RESULT) \
	asm volatile( \
	"swi %a1     \n\t" \
	"mov %0,r0  \n\t" \
	: "=r" (MY_RESULT) : "I" (MY_SWI_ID) : "r0", "lr" )

#define MY_SWI_CALL_PARAM(MY_SWI_ID, MY_PARAM, MY_RESULT) \
	asm volatile( \
	"mov r0,%1  \t\n" \
	"swi %a2     \n\t" \
	"mov %0,r0  \n\t" \
	: "=r" (MY_RESULT) : "r" (MY_PARAM), "I" (MY_SWI_ID) : "r0", "lr" )

static inline unsigned long IntGetCPSR(void)
{	
	unsigned long res; 
	MY_SWI_CALL_RES(SWI_NUM_GET_CPSR, res);
	return res;
}

static inline unsigned long IntDisable(void)
{
	unsigned long res; 
	MY_SWI_CALL_RES(SWI_NUM_IRQ_DIS, res);
	return res;
}

static inline unsigned long IntEnable(void) {
	unsigned long res; 
	MY_SWI_CALL_RES(SWI_NUM_IRQ_EN, res);
	return res;
}

static inline void IntRestore(unsigned long oldstate)
{
	unsigned long res; 
	MY_SWI_CALL_PARAM(SWI_NUM_IRQ_REST, oldstate, res);
}

static inline unsigned long FiqDisable(void)
{
	unsigned long res; 
	MY_SWI_CALL_RES(SWI_NUM_FIQ_DIS, res);
	return res;
}

static inline unsigned long FiqEnable(void) {
	unsigned long res; 
	MY_SWI_CALL_RES(SWI_NUM_FIQ_EN, res);
	return res;
}

static inline void FiqRestore(unsigned long oldstate)
{
	unsigned long res; 
	MY_SWI_CALL_PARAM(SWI_NUM_FIQ_REST, oldstate, res);
}



#if 0
static inline unsigned long do_SWI2 (int swi_num, 
	unsigned long par0,
	unsigned long par1,
	unsigned long par2,
	unsigned long par3)
{
	unsigned long ret_val;

	register unsigned long reg0 asm("r0") = par0;
	register unsigned long reg1 asm("r1") = par1;
	register unsigned long reg2 asm("r2") = par2;
	register unsigned long reg3 asm("r3") = par3;

	asm volatile (
		/* "mov r0, %2 \n\t" \
		"mov r1, %3 \n\t" \
		"mov r2, %4 \n\t" \
		"mov r3, %5 \n\t" \ */
		"swi %a1    \n\t" \
        "mov %0, r0 \n\t" \
		:	"=r" (ret_val) /* Outp */
		:	"I" ((const int)swi_num) // , 
		/*	"r" (reg0), "r" (reg1), "r" (reg2), "r" (reg3) */ /* Inp */
		:	"r0", "ip", "lr", "memory", "cc" /* Clobber */ 
	);

	return ret_val;
}
#endif

#if 0
static inline unsigned long do_SWI( const int swi_num, 
	unsigned long par0,
	unsigned long par1,
	unsigned long par2,
	unsigned long par3 )
{
	unsigned long ret_val;

	asm volatile (
		"mov r0, %2 \n\t" \
		"mov r1, %3 \n\t" \
		"mov r2, %4 \n\t" \
		"mov r3, %5 \n\t" \
		"swi %a1    \n\t" \
		"mov %0, r0 \n\t" \
		:	"=r" (ret_val)
		:	"I" ((const int)swi_num), 
			"r" (par0), "r" (par1), "r" (par2), "r" (par3)
		:	"r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
	);

	return ret_val;
}
#endif


#endif






