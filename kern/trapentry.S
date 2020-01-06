/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

  // Za part A: preuzeto iz inc/trap.h

  TRAPHANDLER_NOEC(DIVIDE, 0) 
  TRAPHANDLER_NOEC(DEBUG, 1)
  TRAPHANDLER_NOEC(NMI, 2)
  TRAPHANDLER_NOEC(BRKPT, 3)
  TRAPHANDLER_NOEC(OFLOW, 4)
  TRAPHANDLER_NOEC(BOUND, 5)
  TRAPHANDLER_NOEC(ILLOP, 6)
  TRAPHANDLER_NOEC(DEVICE, 7)
  TRAPHANDLER(DBLFLT, 8)
  TRAPHANDLER(TSS, 10)
  TRAPHANDLER(SEGNP, 11)
  TRAPHANDLER(STACK, 12)
  TRAPHANDLER(GPFLT, 13)
  TRAPHANDLER(PGFLT, 14)
  TRAPHANDLER_NOEC(FPERR, 16)
  TRAPHANDLER(ALIGN, 17)
  TRAPHANDLER_NOEC(MCHK, 18)
  TRAPHANDLER_NOEC(SIMDERR, 19) 

  TRAPHANDLER_NOEC(SYSCALL, T_SYSCALL)
/*
 * Lab 3: Your code here for _alltraps
 */

  _alltraps:
    pushl %ds
    pushl %es
    pushal
    movw $GD_KD, %ax
    movw %ax, %ds
    movw %ax, %es
    pushl %esp
    call trap

