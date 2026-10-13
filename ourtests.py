#!/usr/bin/env python3
"""Run our tests from our_tests/ (or a given folder).

Each test is a pair: name.c (compiled by us) and name_driver.c (compiled by gcc).
The driver's exit code is the result: 0 means pass.
"""
import sys
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
COMPILER = ROOT / "build" / "c_compiler"
GCC = ["riscv32-unknown-elf-gcc", "-march=rv32gc", "-mabi=ilp32d"]


def run(cmd):
    return subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode


def run_test(src, tmp):
    driver = src.with_name(f"{src.stem}_driver.c")
    asm, obj, binary = (tmp / f"{src.stem}{ext}" for ext in (".s", ".o", ""))

    if run([str(COMPILER), "-S", str(src), "-o", str(asm)]) != 0:
        return "compile"
    if run(GCC + ["-c", str(asm), "-o", str(obj)]) != 0:
        return "assemble"
    if run(GCC + ["-static", str(obj), str(driver), "-o", str(binary)]) != 0:
        return "link"
    code = run(["spike", "--isa=rv32gc", "pk", str(binary)])
    return None if code == 0 else f"exit={code}"


def main():
    folder = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "our_tests"
    if run(["make", "-C", str(ROOT)]) != 0:
        sys.exit("build failed")

    tests = sorted(p for p in folder.rglob("*.c") if not p.stem.endswith("_driver"))
    passed = 0
    with tempfile.TemporaryDirectory() as tmp:
        for src in tests:
            err = run_test(src, Path(tmp))
            name = src.relative_to(folder)
            print(f"PASS            {name}" if err is None else f"FAIL [{err}]  {name}")
            passed += err is None

    print(f"--- {passed} passed, {len(tests) - passed} failed ---")
    sys.exit(0 if passed == len(tests) else 1)


if __name__ == "__main__":
    main()
