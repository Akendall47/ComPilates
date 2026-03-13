.text
.globl f
f:
addi    sp, sp, -32
sw      ra, 28(sp)
sw      s0, 24(sp)
addi    s0, sp, 32
li      a0, 0
  sw a0, -12(s0)
  addi    a0, s0, -12
  sw a0, -16(s0)
  addi    a0, s0, -16
  sw a0, -20(s0)
li      a0, 55
  mv      t2, a0
  lw a0, -20(s0)
  lw      a0, 0(a0)
  sw      t2, 0(a0)
  lw a0, -12(s0)
lw      ra, 28(sp)
lw      s0, 24(sp)
addi    sp, sp, 32
ret
