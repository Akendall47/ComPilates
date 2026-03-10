;
; ptr_chain.c  —  **pp read (double pointer dereference)
;
;   int f() {
;       int x;    int *p;    int **pp;
;       x = 7;
;       p = &x;
;       pp = &p;
;       return **pp;
;   }
;

; ---US -----------
f:
  addi  sp, sp, -32
  sw    ra, 28(sp)
  sw    s0, 24(sp)
  addi  s0, sp, 32

  li    a0, 7
  sw    a0, -12(s0)        ; x = 7

  addi  a0, s0, -12
  sw    a0, -16(s0)        ; p = &x  (store address of x)

  addi  a0, s0, -16
  sw    a0, -20(s0)        ; pp = &p  (store address of p)

  lw    a0, -20(s0)        ; a0 = pp  (address of p)
  lw    a0, 0(a0)          ; a0 = *pp = p  (address of x)
  lw    a0, 0(a0)          ; a0 = **pp = x = 7

  lw    ra, 28(sp)
  lw    s0, 24(sp)
  addi  sp, sp, 32
  ret

; --- GCC -O0 ---
; (identical logic, uses a5 as scratch instead of a0 throughout,
;  then mv a0, a5 at the end — functionally the same)
;
;   lw  a5,-20(s0)         ; a5 = pp
;   lw  a5,0(a5)           ; a5 = *pp = p
;   lw  a5,0(a5)           ; a5 = **pp = x
;   mv  a0,a5              ; return value

; KEY INSIGHT:
;   Each level of * = one extra "lw reg, 0(reg)" in a chain.
;   int*   : 1x lw
;   int**  : 2x lw
;   int*** : 3x lw  (and so on)
