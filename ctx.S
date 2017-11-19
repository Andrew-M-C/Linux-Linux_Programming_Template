#
# Andrew Chang
# 2017/11/16
#
#

#if defined (__amd64__) || defined(__x68_64__)
##
	.text
	.align 4
	.globl asm_swap
	.type asm_swap, @function
asm_swap:
	push %rbp
	mov %rsp, %rbp
	sub $0x20, %rsp
	push %r12
	push %r13

# here is function code
	mov (%rdi), %r12
	mov (%rsi), %r13
	mov %r13, (%rdi)
	mov %r12, (%rsi)

	pop %r13
	pop %r12
	mov %rbp, %rsp
	pop %rbp
	ret

#elif defined(__i386__)
#error i386 Upsupported
#else
#error Upsupported CPU archetecture
#endif

