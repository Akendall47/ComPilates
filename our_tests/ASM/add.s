.text
.globl f
f:
addi    sp, sp, -32
sw      ra, 28(sp)
sw      s0, 24(sp)
addi    s0, sp, 32
  sw a0, -12(s0)
  sw a1, -16(s0)
lw      a0, -12(s0)
  addi sp, sp, -4
  sw a0, 0(sp)
lw      a0, -16(s0)
  lw t1, 0(sp)
  addi sp, sp, 4
  add a0, t1, a0
lw      ra, 28(sp)
lw      s0, 24(sp)
addi    sp, sp, 32
ret
