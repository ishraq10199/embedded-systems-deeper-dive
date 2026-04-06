#include "faulthandlers.h"
#include "uart.h"

typedef struct {

	/* From figure 12 (page 43) of PM0214 */
	/* Layout of the Exception frame */
	uint32_t r0, r1, r2, r3;
	uint32_t r12, lr, pc, xpsr;

} ExceptionFrame;


#define UFSR_DIV_BY_ZERO		(1U << 25)

void div_by_zero_handler(ExceptionFrame *frame) {
	printf(
		"Encountered a division by zero error at PC=0x%08lX. Skipping instruction.\r\n",
		frame->pc
	);

	/* First, we need to clear the fault flag (write-1-to-clear) so it doesn't retrigger */
	SCB->CFSR = UFSR_DIV_BY_ZERO;

	/* Now, frame->pc will point to the exception-causing instruction */
	/* When the handler returns, it will be executed again */
	/* So, we need to skip it by advancing the pc value */

	/* However, the instruction is a Thumb2 instruction */
	/* Which can be either 16-bit or 32-bit */
	/* The first 5 bits of the Thumb2 instruction denote which one it is */
	/*
	 * From the `ARM DDI 0308D` documentation, section 3.1:
	 *
	 * The first half word of the Thumb2 instruction, specifically bits [15:11]
	 * are needed to decode the instruction width. i.e.:
	 *
	 * 11100			=> Thumb2 16-bit unconditional branch instruction
	 * 11101 ~ 11111	=> Thumb2 32-bit instructions
	 * Remaining range 	=> Thumb2 16-bit instructions
	 *
	 * So, to put it simply, we can just get the 5 bits starting from MSB of the pc value
	 * Then check whether its in the range of [0x1D ~ 0x1F]
	 * If it is, its a 32-bit instruction, and if its not, its a 16-bit instruction
	 * */

	uint16_t faulting_instr = *(uint16_t *)(frame->pc);

	// We can just check if its more than 1D (max range of 1F is implied)
	if ((faulting_instr >> 11) >= 0x1D) {
		/* Its a 32-bit instruction, so we advance by 4 bytes */
		frame->pc += 4;
	} else {
		/* Otherwise, its a 16-bit instruction, so we advance by 2 bytes */
		frame->pc += 2;
	}

	/* After this, the execution will resume at the updated frame->pc */

}

void default_handler(ExceptionFrame *frame) {
	printf(
		"Unexpected HardFault! PC=0x%08lX \t CFSR=0x%08lX\r\n",
		frame->pc, SCB->CFSR
	);

	/* Spin here forever, since there is no safe way to recover from an unknown HardFault */
}

/* This handler catches ALL hardfaults */
/* Documentation for the fault registers is in PM0214 */
/* The __attribute__ ((naked)) part is a GCC-specific thing */
/* which is not part of the original handler */
/* We need to add this in order to make graceful exits possible */
__attribute__((naked)) void HardFault_Handler(void) {

	/* As soon as we encounter a hard fault, we enter here
	 * The ((naked)) attribute means that no prologue or epilogue is present here
	 * Here, we identify where our Exception frame was stored, i.e. in the MSP or PSP
	 * After identification, we simply call the HardFault_Handler_C function with the
	 * Exception frame as the first argument of the function
	 *
	 * It is necessary to identify the location of the Exception frame,
	 * because if we address MSP or PSP incorrectly, it will result in garbage values
	 *
	 * Explanation of the assembly code:
	 *
	 * TST lr #4
	 *
	 * 	+ EXC_RETURN is loaded into lr at the time of exception entry
	 * 	+ EXC_RETURN is a 32-bit value that has bits [31:5] set to 1
	 * 	+ The lower bits indicate where the Exception frame is loaded
	 * 	+	------------------------------------------------------------
	 * 	+	EXC_RETURN		...	3 2 1 0		Description
	 * 	+   ------------------------------------------------------------
	 * 	+ 	0xFFFFFFF1	    ...	0 0 0 1   → FPU not used, MSP, handler mode
	 *  +	0xFFFFFFF9	    ...	1 0 0 1   → FPU not used, MSP, thread mode
	 *  +	0xFFFFFFFD	    ...	1 1 0 1   → FPU not used, PSP, thread mode
	 *	+ In our division by zero case, for example, we have lr => 0xFFFFFFF9
	 *	+ This means &-ing #4 with it gives us 0 (TST => bitwise AND + discard the result)
	 *	+ So our Exception Frame is in MSP
	 *
	 * ITE EQ
	 *
	 * 	+ This is to check what the result of the TST was
	 * 	+ If it was equal (i.e. the Zero flag is set), we branch
	 *
	 * MRSEQ r0, MSP
	 *
	 * 	+ Move to (GP) Register from Special (Register) if Equal
	 * 	+ i.e. if our Zero flag was set, this move instruction happens
	 * 	+ Since having the Zero flag set means that our Exception frame is in MSP,
	 * 	+ we move our MSP value (a stack pointer) to the r0 register
	 * 	+ r0 is used as the first argument in the next function call
	 *
	 * MRSNE r0, PSP
	 *
	 *  + Move to (GP) Register from Special (Register) if Not Equal
	 *  + i.e. we do the same thing as the MRSEQ, but for PSP instead of MSP
	 *
	 * B HardFault_Handler_C
	 *
	 * + B (Branch) is an unconditional jump to the HardFault_Handler_C function
	 * + This is not the same as the BL (Branch with Link), which overrites the lr
	 * + Here, the argument is the stack pointer address we put into r0
	 * + from our last instruction (MRSEQ/MRSNE)
	 */


	__asm volatile (

			"TST	lr, #4					\n"
			"ITE	EQ						\n"
			"MRSEQ	r0, MSP					\n"
			"MRSNE	r0, PSP					\n"
			"B		HardFault_Handler_C		\n"

	);
}

void HardFault_Handler_C(ExceptionFrame *frame) {


	if (SCB->CFSR & UFSR_DIV_BY_ZERO) {

		div_by_zero_handler(frame);

	} else {

		default_handler(frame);

	}

}
