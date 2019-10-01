	.arch armv7-a
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"cut_struct_disasm.c"
	.text
	.align	2
	.global	cut_struct
	.syntax unified
	.arm
	.fpu softvfp
	.type	cut_struct, %function
cut_struct:
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	mov	ip, r0
	push	{r4, lr}
	ldrb	r4, [ip]	@ zero_extendqisi2
	mov	r0, lr
	ldr	lr, [sp, #8]
	strb	r4, [r1]
	ldr	r1, [ip, #1]	@ unaligned
	str	r1, [r2]
	ldrsh	r2, [ip, #5]	@ unaligned
	strh	r2, [r3]	@ movhi
	ldrb	r3, [ip, #7]	@ zero_extendqisi2
	strb	r3, [lr]
	pop	{r4, pc}
	.size	cut_struct, .-cut_struct
	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
	.section	.note.GNU-stack,"",%progbits
