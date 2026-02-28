# source: tests/array/index_variable.c
# int f() { int i; int x[8]; for(i=0; i<8; i++){ x[i]=i; } return x[4]; }
#general path for x[i] store, const fast path for x[4] load

.text
.globl f
f:
addi    sp, sp, -64
sw      ra, 60(sp)
sw      s0, 56(sp)
addi    s0, sp, 64

li      a0, 0
sw      a0, -12(s0)

.Lfor_0:
lw      a0, -12(s0)     # load i
  addi sp, sp, -4       # push lhs i<8
  sw a0, 0(sp)
li      a0, 8
  lw t1, 0(sp)
  addi sp, sp, 4
  slt a0, t1, a0
beqz    a0, .Lend_0

# x[i] = i, general path; index is variable, must compute address at runtime
lw      a0, -12(s0)
slli    t2, a0, 2       # t2 = i * 4  (slli cheaper than mul)
addi    t2, t2, -44
add     t2, t2, s0
lw      a0, -12(s0)
sw      a0, 0(t2)

# i++
lw      a0, -12(s0)
addi    a0, a0, 1
sw      a0, -12(s0)
j       .Lfor_0

.Lend_0:
# return x[4] folded at compile time
lw      a0, -28(s0)     # 1 instr; x[4] direct load, offset -28 baked in

lw      ra, 60(sp)
lw      s0, 56(sp)
addi    sp, sp, 64
ret
