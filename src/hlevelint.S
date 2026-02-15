// This file was copied from .\hli\hlevelint.S.
// The file is read only. Do not try to edit this file.
// Edit the original one in the hli-folder.
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

	// --- auto generated ---
	addi	sp, sp, -16
	s32i	a7,sp,0
	s32i	a9,sp,4
	s32i	a10,sp,8
	s32i	a11,sp,12
	// ----------------------

	call0 	hlintC_main

	// --- auto generated ---
	l32i	a7,sp,0
	l32i	a9,sp,4
	l32i	a10,sp,8
	l32i	a11,sp,12
	addi	sp, sp, 16
	// ----------------------

	mov		a0,sp			// a0 = a1(sp)
	l32i	sp,a0,0			// restore sp from stack
    rsr     a0, EXCSAVE_5	// restore a0
    rfi     5

    .global ld_include_xt_highint5
ld_include_xt_highint5:
