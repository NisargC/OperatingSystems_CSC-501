.section .data
.section .text
.globl zfunction

zfunction:
    push %ebp
    mov %esp, %ebp			# Getting the input variable into ebp
	mov 8(%ebp), %eax
    and $0xFFFC01FF, %eax	# Blanking out the 9th to 17th bit that are not needed
	sar $8, %eax			# Shifting the parameter  by 8 bits to the right
							# and setting the starting bits to 1
    pop %ebp				# Returning the value to the calling process
	ret
