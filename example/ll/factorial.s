	.text
	.file	"factorial.c"
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movl	$0, -12(%rbp)
	movl	$1, -8(%rbp)
	movl	$1, -4(%rbp)
.LBB0_1:                                # =>This Inner Loop Header: Depth=1
	cmpl	$6, -4(%rbp)
	jge	.LBB0_4
# %bb.2:                                #   in Loop: Header=BB0_1 Depth=1
	movl	-4(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -8(%rbp)
# %bb.3:                                #   in Loop: Header=BB0_1 Depth=1
	movl	-4(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -4(%rbp)
	jmp	.LBB0_1
.LBB0_4:
	movl	-8(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.ident	"Ubuntu clang version 18.1.3 (1ubuntu1)"
	.section	".note.GNU-stack","",@progbits
