# source: tests/array/index_constant.c
# int f() { int x[8]; x[0]=23; return x[0]; }
# const index fast path on both store and load

.text
.globl f
f:
addi    sp, sp, -48     # frame: 8 (ra+s0) + 8 (pre-pass scalar) + 32 (x[8])
sw      ra, 44(sp)      # save return address
sw      s0, 40(sp)      # save old frame pointer
addi    s0, sp, 48      # s0 = frame pointer


# x[0] = 23
# OPTIMISED: offset folded at compile time
# base of x[] = -44 (past ra/s0 saves and pre-pass slot), x[0] = -44 + 0*4 = -44... wait -40
addi    t2, s0, -40     # 1 instr: address of x[0] folded to -40
li      a0, 23
sw      a0, 0(t2)


# return x[0]
# OPTIMISED: ArraySubscript fast path, no t2 at all, offset straight into lw
lw      a0, -40(s0)     # 1 instr: direct load, offset baked into instruction

lw      ra, 44(sp)
lw      s0, 40(sp)
addi    sp, sp, 48
ret
