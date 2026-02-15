#include <xtensa/coreasm.h>

    .data
    .space      128
hli_top_of_stack:

    .section .iram1,"ax"
    .global     xt_highint5
    .type       xt_highint5,@function
    .align      4
xt_highint5:
	// a0 holds return address
	// a0 is saved in EXCSAVE_5
	// a1 = used for SP

    movi	a0, hli_top_of_stack-4 	
	s32i	sp,a0, 0		// save sp on stack
	mov		sp,a0			// SP = a0	

	// [REG_SAVE]			// from here compile_hli.py will insert all used registers to svae

	call0 	hlintC_main

	// [REG_RESTORE]

	mov		a0,sp			// a0 = a1(sp)
	l32i	sp,a0,0			// restore sp from stack
    rsr     a0, EXCSAVE_5	// restore a0
    rfi     5

    .global ld_include_xt_highint5
ld_include_xt_highint5:
