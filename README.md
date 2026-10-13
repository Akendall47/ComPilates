<div align="center">
  <img src="dashboard/Images/transparent_logo_compilates.png" alt="ComPilates Logo" width="600"/>

  # ComPilates:  a flexible C90 to RISC-V Compiler

  **Archie & Sam** | Your ComPilates Instructors | Imperial College London, Compilers Coursework 2025/26
</div>

---

## What it does

Compiles a subset of **C90** to **RISC-V assembly** (RV32IM), targeting Ubuntu 22.04.

Built using Flex (lexer), Bison (parser), and a hand written AST with RISC-V code generation.


## Building

Needs `g++`, `flex` and `bison`:

```bash
make
build/c_compiler -S input.c -o output.s
```

## Testing

Our tests live in `our_tests/`. Each test is a `name.c` compiled by our compiler plus a `name_driver.c` that calls it and exits 0 on success. Running them needs the RISC-V GNU toolchain (`riscv32-unknown-elf-gcc`) and `spike` with `pk`. The `Dockerfile` sets these up.

```bash
python3 ourtests.py              # all of our_tests/
python3 ourtests.py our_tests/pointer
```

## Project dashboard

`dashboard/` holds the task tracker and progress dashboard we built to plan the project (a GitHub Action, an AWS Lambda and DynamoDB, with a static page on S3).

## Credits

The lexer/parser/AST starter skeleton and build setup came from the course template. Everything else, including code generation, types, pointers, arrays, structs, strings, floats, control flow, functions, our tests and the dashboard, was written by us.
