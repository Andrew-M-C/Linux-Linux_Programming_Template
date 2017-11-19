#
# Andrew Chang
# 2017/11/16
#
#

#if defined (__amd64__) || defined(__x68_64__)
##
	.text
	.align 4
	.globl asm_invoke_func
	.type asm_invoke_func, @function
asm_invoke_func:
#	push %rbp
#	mov %rsp, %rbp
#	sub $0x20, %rsp
#	push %r12
#	push %r13

# here is function code
	
#	mov %rbp, %rsp
#	pop %rbp
	jmp *%rdi

#	pop %r13
#	pop %r12
#	mov %rbp, %rsp
#	pop %rbp
#	retq

#elif defined(__i386__)
#error i386 Upsupported
#else
#error Upsupported CPU archetecture
#endif

