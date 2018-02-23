#!/usr/bin/env python3

import sys
import os
import string
import subprocess

EPSILON = .1
EXTRA_ARGS = []
success = True;

# Parse an s-expression from a string.
#
# This doesn't do much error reporting, so probably only use it on
# machine generated s-expressions which you're pretty sure are
# correct.
def parseSexp(sexpString):
    listStack = list()
    curToken = ""
    for c in sexpString:
        if (c == '('):
            listStack.append(list())
        elif (c == ')'):
            finishedList = listStack.pop()
            if (curToken != ""):
                finishedList.append(curToken)
            if (len(listStack) == 0):
                return finishedList
            curToken = ""
            listStack[len(listStack) - 1].append(finishedList)
        elif (c == ' '):
            if (curToken != ""):
                listStack[len(listStack) - 1].append(curToken)
            curToken = ""
        else:
            curToken += c
    print("Trouble parsing! Unclosed paren. Exiting...")
    sys.exit(1)

def isFloat(floatString):
    if list.__instancecheck__(floatString):
        return False
    for c in floatString:
        if not (c.isnumeric() or c == '.'):
            return False
    return True

# Look up a field in an assoc list.
def lookupField(alst, fieldName):
    for entry in alst:
        if entry[0] == fieldName:
            return entry[1]
    print("Couldn't find field {}! Exiting...".format(fieldName))
    sys.exit(1)

def checkMatch(expected, actual):
    if list.__instancecheck__(expected):
        if not list.__instancecheck__(actual):
            return False
        else:
            for (eitem, aitem) in zip(expected, actual):
                if not checkMatch(eitem, aitem):
                    return False
            return True
    elif isFloat(expected):
        # Treat it as a float, and check to make sure
        # the actual and expected values "almost" match.
        if abs(float(expected) - float(actual)) > EPSILON:
            print("{} mismatch (number): expected {}, got {}".format(fieldName, expectedResult, actualResult))
            success = False
    else:
        if actualResult != expectedResult:
            print("{} mismatch: expected {}, got {}".format(fieldName, expectedResult, actualResult))
            success = False
    

def checkFile(name, ignoreProps):
    with open(name) as actual:
        with open(name + ".expected") as expected:
            for (actualLine, expectedLine) in zip(actual, expected):
                actualResults = parseSexp(actualLine)
                expectedResults = parseSexp(expectedLine)
                for entry in expectedResults:
                    fieldName = entry[0]
                    if not (fieldName in ignoreProps):
                        expectedResult = entry[1]
                        actualResult = lookupField(actualResults, fieldName)
                        # If the string starts with a digit...
                        if isFloat(expectedResult):
                            # Treat it as a float, and check to make sure
                            # the actual and expected values "almost" match.
                            if abs(float(expectedResult) - float(actualResult)) > EPSILON:
                                print("{} mismatch (number): expected {}, got {}".format(fieldName, expectedResult, actualResult))
                                success = False
                        else:
                            if actualResult != expectedResult:
                                print("{} mismatch: expected {}, got {}".format(fieldName, expectedResult, actualResult))
                                success = False

def test(prog, ignoreProps):
    command = "./valgrind/herbgrind-install/bin/valgrind --tool=herbgrind {} {}".format(EXTRA_ARGS, prog)
    print("Calling `{}`".format(command))
    hgproc = subprocess.run(
        ["./valgrind/herbgrind-install/bin/valgrind", "--tool=herbgrind"] + EXTRA_ARGS + [prog],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if hgproc.returncode:
        print("Command failed (status {}).".format(hgproc.returncode))
        success = False
    checkFile("bench/{}-errors.gh".format(prog), ignoreProps)
    return success

if __name__ == "__main__":
    ignoreProps = ["instr-addr"]

    for arg in sys.argv[1:]:
        success = test(arg, ignoreProps)
        if not success:
            sys.exit(success)
