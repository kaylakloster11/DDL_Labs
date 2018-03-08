/*
 * translateFib.s
 *
 *  Created on: Feb 1, 2018
 *      Author: Kayla and Mallory
 */

 	.syntax unified
 	.cpu cortex-m0
 	.align	1
 	.global	fibonacci
 	.thumb
 	.thumb_func

//recursive fibonacci
fibonacci:

	push {r7,lr}
	sub sp, sp, #8
	add r7, sp, #0

	mov r3, r0
	cmp r3, #2
	bne get_Val

	movs r0,#1
	str r0, [r7,#4]
	b recurse

	get_Val:
	str r0, [r7,#4]
	cmp r3, #1
	ble end_prgm
	subs r3,#1
	movs r0,r3
	bl fibonacci


	movs r2,r0
	movs r4,r7
	subs r4,#32
	ldr r3,[r4,#4]
	adds r2,r3
	str r2,[r7,#4]

	mov r0,r2
	bl end_prgm

	recurse:
		bl fibonacci

	end_prgm:
		mov sp, r7
		add sp, sp,#8
		pop {r7,pc}
