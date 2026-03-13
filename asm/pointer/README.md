# Pointer Chaining Assembly Notes


## Key Rule
Each `*` in an expression = one extra `lw reg, 0(reg)` in a chain:
```
*p   → lw a0, offset(s0)       ; load p's value (1 load)
**pp → lw a0, offset(s0)       ; load pp's value
       lw a0, 0(a0)            ; follow first pointer (2 loads)
```

## Write Through Double Pointer (`**pp = 55`)
mst save the rhs value before evaluating the pointer chain, can mess with a0
```
li  a0, 55          ; rhs value
mv  t2, a0          ; save it
lw  a0, -20(s0)     ; pp
lw  a0, 0(a0)       ; *pp = address of x
sw  t2, 0(a0)       ; x = 55
```
GCC does the same, load pointer chain first, then store.
