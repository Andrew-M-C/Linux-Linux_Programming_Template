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

#################################################
##
	.text
	.align 4
	.globl asm_init_context
	.type asm_init_context, @function
asm_init_context:
	movq %rbx, (%rdi)
	movq %rsp, 8(%rdi)
	movq %rbp, 16(%rdi)
	movq %r12, 24(%rdi)
	movq %r13, 32(%rdi)
	movq %r14, 40(%rdi)
	movq %r15, 48(%rdi)
	retq

#################################################
##
	.text
	.align 4
	.globl asm_save_context
	.type asm_save_context, @function
asm_save_context:

	movq %rbx, (%rdi)
	movq %rsp, 8(%rdi)
	movq %rbp, 16(%rdi)
	movq %r12, 24(%rdi)
	movq %r13, 32(%rdi)
	movq %r14, 40(%rdi)
	movq %r15, 48(%rdi)
	pop 64(%rdi)

	retq

#################################################
##
	.text
	.align 4
	.globl asm_save_main_and_go
	.type asm_save_main_and_go, @function
asm_save_main_and_go:

	movq %rbx, (%rdi)
	movq %rsp, 8(%rdi)
	movq %rbp, 16(%rdi)
	movq %r12, 24(%rdi)
	movq %r13, 32(%rdi)
	movq %r14, 40(%rdi)
	movq %r15, 48(%rdi)

	movl %esi, %eax
	movq (%rsi), %rbx
	movq 8(%rsi), %rsp
	movq 16(%rsi), %rbp
	movq 24(%rsi), %r12
	movq 32(%rsi), %r13
	movq 40(%rsi), %r14
	movq 48(%rsi), %r15
	movq 56(%rsi), %rdx
	pop %rdx
	movq %rdx, 56(%rsi)
#	push %rdx
	jmp 64(%rsi)

#################################################
##
	.text
	.align 4
	.globl asm_get_stack_addr
	.type asm_get_stack_addr, @function
asm_get_stack_addr:
	movq %rsp, %rax
	add $0x0, %rax
	retq


#elif defined(__i386__)
#error i386 Upsupported
#else
#error Upsupported CPU archetecture
#endif

