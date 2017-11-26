#
# Copyright (c) 2017, Andrew Chang, All rights reserved.
# File Name: AMCCoroutineAsm.S
# Create Time: 2017-11-23
# License: BSD 2-clause "Simplified" License
#
# History:
#     2017-11-23: File created as AMCCoroutineAsm.S
#

#if defined (__amd64__) || defined(__x68_64__)


#################################################
##
	.text
	.align 4
	.globl AMCGetStackAddr
	.type AMCGetStackAddr, @function

AMCGetStackAddr:
	movq %rsp, %rax
	addq $16, %rax
	retq

#################################################
##
	.text
	.align 4
	.globl asm_amc_coroutine_dump
	.type asm_amc_coroutine_dump, @function

asm_amc_coroutine_dump:
	movq %rbx, (%rdi)
	movq %rsp, 8(%rdi)
	movq %rbp, 16(%rdi)
	movq %r12, 24(%rdi)
	movq %r13, 32(%rdi)
	movq %r14, 40(%rdi)
	movq %r15, 48(%rdi)
	movq 16(%rsp), %rsi
	movq %rsi, 56(%rdi)
	retq

#################################################
##
	.text
	.align 4
	.globl asm_amc_coroutine_restore
	.type asm_amc_coroutine_restore, @function

asm_amc_coroutine_restore:
	movq (%rdi), %rbx
	movq 8(%rdi), %rsp
	movq 16(%rdi), %rbp
	movq 24(%rdi), %r12
	movq 32(%rdi), %r13
	movq 40(%rdi), %r14
	movq 48(%rdi), %r15
	pushq 64(%rdi)
	jmp 56(%rdi) 

#################################################
##
	.text
	.align 4
	.globl asm_amc_coroutine_enter
	.type asm_amc_coroutine_enter, @function

asm_amc_coroutine_enter:
	movq (%rdi), %rbx
	movq 8(%rdi), %rsp
	movq 16(%rdi), %rbp
	push 64(%rdi)		# create a function return point
	jmp 56(%rdi)

#################################################
##
	.text
	.align 4
	.globl asm_amc_coroutine_switch_sp_rip_to
	.type asm_amc_coroutine_switch_sp_rip_to, @function

asm_amc_coroutine_switch_sp_rip_to:
	popq %rsi		# get return address
	movq (%rdi), %rbx
	movq 8(%rdi), %rsp
	movq 16(%rdi), %rbp
	movq 24(%rdi), %r12
	movq 32(%rdi), %r13
	movq 40(%rdi), %r14
	movq 48(%rdi), %r15
	pushq 8(%rdi)
	push %rsi
	retq

#################################################
##
	.text
	.align 4
	.globl asm_amc_coroutine_return_to_main
	.type asm_amc_coroutine_return_to_main, @function

asm_amc_coroutine_return_to_main:
	popq %rsi			# ignore return address for this function, this will redirect.
	movq (%rdi), %rbx
	movq 8(%rdi), %rsp
	movq 16(%rdi), %rbp
	movq 24(%rdi), %r12
	movq 32(%rdi), %r13
	movq 40(%rdi), %r14
	movq 48(%rdi), %r15
	leaveq
	retq

#elif defined(__i386__)
#error i386 Upsupported
#else
#error Upsupported CPU archetecture
#endif

