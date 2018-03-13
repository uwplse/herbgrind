#!/usr/bin/env python3

import subprocess
import sys
import re
HEX_RE = re.compile(r"\(instr-addr [0-9a-fA-F]+\)")

def compare_results(actual, expected):
    return HEX_RE.sub("<addr>", actual) == HEX_RE.sub("<addr>", expected)

def test(prog):
    global success
    command = ["./valgrind/herbgrind-install/bin/valgrind", "--tool=herbgrind", "--output-sexp", prog]
    print("Calling `{}`...".format(" ".join(command)), end="")
    status = subprocess.call(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if status:
        print("Command failed (status {}).".format(status))
        success = False
    with open(prog + ".gh") as actual, open(prog + ".expected") as expected:
        actual_text, expected_text = actual.read(), expected.read()
    if compare_results(actual_text, expected_text):
        print("Outputs match.")
        return True
    else:
        print("Outputs do not match!")
        print("Actual::", actual_text, sep="\n")
        print("Expected::", expected_text, sep="\n")
        return False

if __name__ == "__main__":
    for arg in sys.argv[1:]:
        if not test(arg):
            sys.exit(1)
