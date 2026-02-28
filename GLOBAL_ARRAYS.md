# Global Arrays note

## Why the current test passes (but is wrong)

`tests/array/declare_global.c`:
```c
int x[8];
int f() { return 11; }
```
Driver just checks `f() == 11` never touches `x`.

Our `ArrayDeclaration::EmitRISC` calls `context.AllocateLocal("x", 32)` at global scope.
This writes an offset into `context.locals` and emits **no assembly** (stream is
discarded). The function `f` compiles fine independently and the test passes by accident.

run `./build/c_compiler -S tests/array/declare_global.c -o /dev/stdout 2>/dev/null` to be sure
## What we currently output

`int x[8];` emits **nothing**,  `ArrayDeclaration::EmitRISC` silently calls `AllocateLocal`
(the `stream` parameter is unnamed/discarded). The test still passes because `f` never touches `x`.

```asm
# our output x[8] completely absent:
.text
.globl f
f:
addi    sp, sp, -16
sw      ra, 12(sp)
sw      s0, 8(sp)
addi    s0, sp, 16
li      a0, 11
lw      ra, 12(sp)
lw      s0, 8(sp)
addi    sp, sp, 16
ret
```

## What Godbolt does

```asm
# correct output, x lives in .bss:
    .bss
    .globl x
    .align 2
x:
    .space 32       # 8 * 4 bytes, zero-inited

    .text
    .globl f
f:
    addi    sp, sp, -16
    sw      ra, 12(sp)
    sw      s0, 8(sp)
    addi    s0, sp, 16
    li      a0, 11
    lw      ra, 12(sp)
    lw      s0, 8(sp)
    addi    sp, sp, 16
    jr      ra
```

## What we did about this issue

Three changes made:

- global tracking
- emit bss, globl, align , .space segments for completness
    - align 2. == 2^2== sw/lw need 4 bytes aligned addrs
- considered lui/addi %hi %lo for globals and ie how to build address from scratch as can't do relative to s0
    - offset from s0 for locals

