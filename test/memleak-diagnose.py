#!/usr/bin/env python3

import argparse
import subprocess
import re
import os
import sys


parser = argparse.ArgumentParser(description="test for memory leaks in a process.")
parser.add_argument("basedir", nargs="?", default=os.getcwd(), help="The base directory to run the binary from.")
parser.add_argument("executable", help="The binary to test herbgrind on.")
parser.add_argument("--print-log", help="Print out the output of the herbgrind command run.\n",
                    default=False, const=True, action='store_const', dest='print_log')

args = parser.parse_args()

print("with executable {}".format(args.executable))

log=subprocess.Popen("/home/alex/bin/timeout -t 4200 -m 1500000 --confess /home/alex/herbie-all/herbgrind/valgrind/herbgrind-install/bin/valgrind --tool=herbgrind --print-moves " + args.executable,
                     shell=True, executable="/bin/bash",
                     stderr=subprocess.PIPE,
                     cwd=args.basedir).communicate()[1]

if args.print_log:
    print(log)

svs_made = [];
svs_freed = [];
for line in log.decode("utf-8").splitlines():
    make_match = re.match("Making shadow value (0x[0-9ABCDEF]*)", line)
    if make_match:
        svs_made.append(make_match.group(1))

    free_match = re.match("Cleaning up shadow value (0x[0-9ABCDEF]*)", line)
    if free_match:
        svs_freed.append(free_match.group(1))

assert (set(svs_freed) <= set(svs_made))

svs_leaked = list(set(svs_made) - set(svs_freed))

print("{} shadow values leaked! ({} made, {} freed)\n".format(len(set(svs_leaked)),
                                                              len(set(svs_made)),
                                                              len(svs_freed)))
