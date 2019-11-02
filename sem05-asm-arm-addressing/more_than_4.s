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
	.file	"more_than_4.c"
	.text
	.align	2
	.global	mega_sum
	.syntax unified
	.arm
	.fpu softvfp
	.type	mega_sum, %function
mega_sum:
	@ args = 8, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	add	r1, r0, r1
	ldr	ip, [sp]
	add	r1, r1, r2
	ldr	r0, [sp, #4]
	add	r1, r1, r3
	add	r1, r1, ip
	add	r0, r1, r0
	bx	lr
	.size	mega_sum, .-mega_sum
	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
	.section	.note.GNU-stack,"",%progbits
