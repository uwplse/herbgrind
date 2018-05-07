#!/usr/bin/env python3

import subprocess
import sys
import re
HEX_RE = re.compile(r"\(instr-addr [0-9a-fA-F]+\)")

def compare_results(actual, expected):
    return HEX_RE.sub("<addr>", actual) == HEX_RE.sub("<addr>", expected)

def test(prog):
    global success
    command = ["./valgrind/herbgrind-install/bin/valgrind", "--tool=herbgrind",
               "--output-sexp", prog]
    print("Calling `{}`...".format(" ".join(command)), end="")
    proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    status = proc.poll()
    full_stderr = stderr.decode('utf-8')
    stderr_lines = full_stderr.splitlines()
    last_stderr = "\n".join(stderr_lines[-200:])
    if status:
        print("Command failed (status {}).".format(status))
        success = False
    try:
        with open(prog + ".gh") as actual, open(prog + ".expected") as expected:
            actual_text, expected_text = actual.read(), expected.read()
    except:
        print("Cannot find output file {}!".format(prog + ".gh"),
              "stdout::", stdout.decode('utf-8'),
              "stderr::", last_stderr,
              sep="\n")
        return False
    if compare_results(actual_text, expected_text):
        print("Outputs match.")
        return True
    else:
        if actual_text == "":
            print("Empty file at {}!".format(prog + ".gh"))
        if expected_text == "":
            print("Empty file at {}!".format(prog + ".expected"))
        print("Outputs do not match!")
        print("Actual::", actual_text, sep="\n")
        print("Expected::", expected_text, sep="\n")
        print("stdout::", stdout.decode('utf-8'), sep="\n")
        print("stderr::", last_stderr, sep="\n")
        return False

if __name__ == "__main__":
    for arg in sys.argv[1:]:
        if not test(arg):
            sys.exit(1)
