#!/bin/bash
# quick test for arrays

cd "$(dirname "$0")"

GCC="riscv32-unknown-elf-gcc"
ARCH="-march=rv32gc"
ABI="-mabi=ilp32d"
PASS=0; FAIL=0

for src in tests/array/*.c; do
  [[ "$src" == *driver* ]] && continue
  name=$(basename "$src" .c)
  driver="tests/array/${name}_driver.c"
  asm="/tmp/arr_${name}.s"
  obj="/tmp/arr_${name}.o"
  bin="/tmp/arr_${name}"

  # Compile with our compiler
  build/c_compiler -S "$src" -o "$asm" 2>/dev/null
  if [[ $? -ne 0 ]]; then
    printf "FAIL [compile]  %s\n" "$name"; ((FAIL++)); continue
  fi

  # Assemble
  $GCC $ARCH $ABI -c "$asm" -o "$obj" 2>/dev/null
  if [[ $? -ne 0 ]]; then
    printf "FAIL [assemble] %s\n" "$name"; ((FAIL++)); continue
  fi

  # Link with driver
  $GCC $ARCH $ABI -static "$obj" "$driver" -o "$bin" 2>/dev/null
  if [[ $? -ne 0 ]]; then
    printf "FAIL [link]     %s\n" "$name"; ((FAIL++)); continue
  fi

  # Simulate with spike
  spike --isa=rv32gc pk "$bin" > /dev/null 2>&1; got=$?
  if [[ "$got" == "0" ]]; then
    printf "PASS            %s\n" "$name"; ((PASS++))
  else
    printf "FAIL [exit=%s]   %s\n" "$got" "$name"; ((FAIL++))
  fi
done

echo "--- ${PASS} passed, ${FAIL} failed ---"
