.globl	_main
_main:
	endbr64
 	pushq	%rbp  # save %rbp on the stack
	movq	%rsp, %rbp # define %rbp for the current function
	# content
	movl $4, %eax
	cltd
	movl $2, %ecx
	idivl %ecx
	movl %eax, -8(%rbp)
	movl -8(%rbp), %eax
	movl %eax, -16(%rbp)
	movl -16(%rbp), %eax
	# epilogue
	popq	 %rbp  # restore %rbp from the stack
	ret  # return to the caller (here the shell)
