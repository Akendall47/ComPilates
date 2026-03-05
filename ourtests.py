#!/usr/bin/env python3
"""Run our custom tests from our_tests/. Delegates to test.py."""
import sys
import subprocess
from pathlib import Path

our_tests = Path(__file__).resolve().parent / "our_tests"
folder = sys.argv[1] if len(sys.argv) > 1 else str(our_tests)

sys.exit(subprocess.run(
    [sys.executable, "test.py", folder] + sys.argv[2:],
    cwd=Path(__file__).resolve().parent
).returncode)
