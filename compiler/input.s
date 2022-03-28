main:
.LFB0:
	endbr64
	movq	%rsp, %rbp
	movb	$48, -6(%rbp)
	movl	$0, -4(%rbp)
	movl	-4(%rbp), %eax
	movl	%eax, %edx
	movzbl	-6(%rbp), %eax
	addl	%edx, %eax
	movb	%al, -5(%rbp)
	movl	$0, %eax
	popq	%rbp
	ret