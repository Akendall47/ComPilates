# source: tests/array/index_expression.c
# int f() {
#   int i; int x[8]; int acc;
#   for(i=8; i<16; i++) { x[i-8]=i; }   // store with expression index
#   acc=0;
#   for(i=0; i<8; i++)  { acc=acc+x[i+0]; } // load with expression index
#   return acc;
# }

#expression index (i-8, i+0) always takes general path

.text
.globl f
f:
addi    sp, sp, -64
sw      ra, 60(sp)
sw      s0, 56(sp)
addi    s0, sp, 64

li      a0, 8
sw      a0, -12(s0)     # i = 8

.Lfor_0:
lw      a0, -12(s0)
  addi sp, sp, -4
  sw a0, 0(sp)
li      a0, 16
  lw t1, 0(sp)
  addi sp, sp, 4
  slt a0, t1, a0        # i < 16
beqz    a0, .Lend_0

# x[i-8] = i, index is BinaryOp(i-8), general path
lw      a0, -12(s0)     # evaluate i-8: load i
  addi sp, sp, -4
  sw a0, 0(sp)
li      a0, 8
  lw t1, 0(sp)
  addi sp, sp, 4
  sub a0, t1, a0        # a0 = i - 8
slli    t2, a0, 2       # t2 = (i-8) *4
addi    t2, t2, -44     # t2 = (i-8)*4 + base
add     t2, t2, s0      # t2 = absolute address of x[i-8]
lw      a0, -12(s0)     # rhs = i
sw      a0, 0(t2)       # x[i-8] = i

lw      a0, -12(s0)
addi    a0, a0, 1
sw      a0, -12(s0)
j       .Lfor_0

.Lend_0:
li      a0, 0
sw      a0, -48(s0)     # acc = 0
li      a0, 0
sw      a0, -12(s0)     # i = 0

.Lfor_1:
lw      a0, -12(s0)
  addi sp, sp, -4
  sw a0, 0(sp)
li      a0, 8
  lw t1, 0(sp)
  addi sp, sp, 4
  slt a0, t1, a0        # i < 8
beqz    a0, .Lend_1


# acc = acc + x[i+0], index is BinaryOp(i+0), general path
# note: i+0 is not simplified,  algebraic simpfification here would be long and useless in real world

lw      a0, -48(s0)     # load acc for outer add
  addi sp, sp, -4
  sw a0, 0(sp)
lw      a0, -12(s0)     # evaluate i+0: load i
  addi sp, sp, -4
  sw a0, 0(sp)
li      a0, 0
  lw t1, 0(sp)
  addi sp, sp, 4
  add a0, t1, a0        # a0 = i + 0
slli    t2, a0, 2       # t2 = (i+0) * 4
addi    t2, t2, -44     # t2 = base + offset
add     t2, t2, s0      # t2 = absolute address of x[i+0]
lw      a0, 0(t2)       # load x[i+0] — general path lw via t2
  lw t1, 0(sp)
  addi sp, sp, 4
  add a0, t1, a0        # a0 = acc + x[i+0]
sw      a0, -48(s0)     # acc = result

lw      a0, -12(s0)
addi    a0, a0, 1
sw      a0, -12(s0)
j       .Lfor_1

.Lend_1:
lw      a0, -48(s0)     # return acc
lw      ra, 60(sp)
lw      s0, 56(sp)
addi    sp, sp, 64
ret
