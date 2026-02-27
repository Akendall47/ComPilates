# RISC-V Register Allocation Plan

## Current Status

Currently all locals are spilled to the stack via `s0`-relative offsets (see `ast_context.hpp`).
Registers are used only transiently (e.g. `a5` as a scratch in `ast_identifier.cpp`).
This document tracks our plan to implement proper register allocation.

---

---

**Possible Allocator Stages**
(nice-to-haves, implement as far as we get )

- current: stack spill; Every local lives at a fixed `s0`-relative offset; `a5` hardcoded as the one scratch register.

- temp free list.

- Register caching: Track which register currently holds which variable; skip the load if it's already in a reg; invalidate on assignment or call

- Liveness-based linear scan

- Graph colouring*

---

---

## RISC-V Register Reference

### Integer Registers

| Register | ABI Name | Role / Convention | Callee-saved? | Current Usage |
|----------|----------|-------------------|---------------|---------------|
| x0       | zero     | Hardwired 0 — read-only | N/A | Available |
| x1       | ra       | Return address | No (caller-saved) | Saved/restored in prologue/epilogue |
| x2       | sp       | Stack pointer | Yes | Managed in prologue/epilogue |
| x3       | gp       | Global pointer | — | Unused |
| x4       | tp       | Thread pointer | — | Unused |
| x5       | t0       | Temporary / scratch | No (caller-saved) | Unused |
| x6       | t1       | Temporary / scratch | No (caller-saved) | Unused |
| x7       | t2       | Temporary / scratch | No (caller-saved) | Unused |
| x8       | s0 / fp  | Frame pointer / saved reg | Yes | Frame pointer — do NOT use for allocation |
| x9       | s1       | Saved register | Yes | Free to use (must save/restore) |
| x10      | a0       | Arg 0 / return value | No (caller-saved) | Return value in `ret` |
| x11      | a1       | Arg 1 | No (caller-saved) | Unused |
| x12      | a2       | Arg 2 | No (caller-saved) | Unused |
| x13      | a3       | Arg 3 | No (caller-saved) | Unused |
| x14      | a4       | Arg 4 | No (caller-saved) | Unused |
| x15      | a5       | Arg 5 / scratch | No (caller-saved) | Used as scratch load reg in Identifier |
| x16      | a6       | Arg 6 | No (caller-saved) | Unused |
| x17      | a7       | Arg 7 | No (caller-saved) | Unused |
| x18      | s2       | Saved register | Yes | Unused |
| x19      | s3       | Saved register | Yes | Unused |
| x20      | s4       | Saved register | Yes | Unused |
| x21      | s5       | Saved register | Yes | Unused |
| x22      | s6       | Saved register | Yes | Unused |
| x23      | s7       | Saved register | Yes | Unused |
| x24      | s8       | Saved register | Yes | Unused |
| x25      | s9       | Saved register | Yes | Unused |
| x26      | s10      | Saved register | Yes | Unused |
| x27      | s11      | Saved register | Yes | Unused |
| x28      | t3       | Temporary | No (caller-saved) | Unused |
| x29      | t4       | Temporary | No (caller-saved) | Unused |
| x30      | t5       | Temporary | No (caller-saved) | Unused |
| x31      | t6       | Temporary | No (caller-saved) | Unused |


