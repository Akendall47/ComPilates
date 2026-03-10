
; ptr_chain_write.c  —  **pp write (store through double pointer)
;
;   int f() {
;       int x;    int *p;    int **pp;
;       x = 0;
;       p = &x;
;       pp = &p;
;       **pp = 55;
;       return x;
;   }
;

; -------US-------------
f:
  addi  sp, sp, -32
  sw    ra, 28(sp)
  sw    s0, 24(sp)
  addi  s0, sp, 32

  li    a0, 0
  sw    a0, -12(s0)        ; x = 0

  addi  a0, s0, -12
  sw    a0, -16(s0)        ; p = &x

  addi  a0, s0, -16
  sw    a0, -20(s0)        ; pp = &p

  ; **pp = 55  (DerefAssign)
  li    a0, 55             ; a0 = rhs value (55)
  mv    t2, a0             ; t2 = 55  (save before ptr eval clobbers a0)
  lw    a0, -20(s0)        ; a0 = pp  (address of p)
  lw    a0, 0(a0)          ; a0 = *pp = p  (address of x)  <-- ptr_->EmitRISC
  sw    t2, 0(a0)          ; *p = 55  i.e. x = 55

  lw    a0, -12(s0)        ; return x (= 55)

  lw    ra, 28(sp)
  lw    s0, 24(sp)
  addi  sp, sp, 32
  ret

; --- GCC -O0---------------
;   lw  a5,-20(s0)         ; a5 = pp
;   lw  a5,0(a5)           ; a5 = *pp = p  (address of x)
;   li  a4,55
;   sw  a4,0(a5)           ; *p = 55  i.e. x = 55
;   lw  a5,-24(s0)         ; return x
;   mv  a0,a5

; KEY INSIGHT — DerefAssign for **pp = val:
;   1. Evaluate rhs -> a0 (the value to store)
;   2. mv t2, a0            save value, ptr eval will clobber a0
;   3. ptr_->EmitRISC()     walks the pointer chain stopping ONE level short
;                           (gives the address to write to, not the final value)
;   4. sw t2, 0(a0)         store
;
; Compare *p = val (single pointer):
;   ptr_ = Identifier("p")  -> EmitRISC loads p's value = address of x  (1 lw)
; Compare **pp = val (double pointer):
;   ptr_ = Deref(Identifier("pp")) -> EmitRISC loads pp then lw -> *pp = address of x  (2 lw)
