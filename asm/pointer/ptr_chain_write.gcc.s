	.file	"ptr_chain_write.c"
	.option nopic
	.attribute arch, "rv32i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zmmul1p0_zaamo1p0_zalrsc1p0_zca1p0_zcd1p0_zcf1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	f
	.type	f, @function
f:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	addi	s0,sp,32
	sw	zero,-24(s0)
	addi	a5,s0,-24
	sw	a5,-28(s0)
	addi	a5,s0,-28
	sw	a5,-20(s0)
	lw	a5,-20(s0)
	lw	a5,0(a5)
	li	a4,55
	sw	a4,0(a5)
	lw	a5,-24(s0)
	mv	a0,a5
	lw	ra,28(sp)
	lw	s0,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	f, .-f
	.ident	"GCC: () 15.2.0"
	.section	.note.GNU-stack,"",@progbits
