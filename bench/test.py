#!/usr/bin/env python3

import subprocess
import sys
import string

EPSILON = .1
EXTRA_ARGS = []
success = True;

# Parse an s-expression from a string.
#
# This doesn't do much error reporting, so probably only use it on
# machine generated s-expressions which you're pretty sure are
# correct.
def parseSexp(sexpString):
    if sexpString.startswith("No marks found!"): return []
    if not sexpString: return []
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
                yield finishedList
                continue
            curToken = ""
            listStack[len(listStack) - 1].append(finishedList)
        elif c.isspace():
            if (curToken != ""):
                listStack[len(listStack) - 1].append(curToken)
            curToken = ""
        else:
            curToken += c
    if listStack:
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
    else:
        raise KeyError("Couldn't find field {}! Exiting...".format(fieldName))

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
    

def checkFile(name, name_expected, ignoreProps):
    global success
    with open(name) as actual, open(name_expected) as expected:
        actualText = actual.read()
        expectedText = expected.read()
        actualResults = list(parseSexp(actualText))
        expectedResults = list(parseSexp(expectedText))
        if len(actualResults) != len(expectedResults):
            print("Expected {} results, got {}".format(len(expectedResults), len(actualResults)));
            success = False
        for entry in expectedResults:
            fieldName = entry[0]
            if not (fieldName in ignoreProps):
                expectedResult = entry[1]
                try:
                    actualResult = lookupField(actualResults, fieldName)
                except KeyError as e:
                    print(str(e))
                    print("\nActual ::")
                    print(actualText)
                    print("\nExpected ::")
                    print(expectedText)
                    sys.exit(1)
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
    global success
    command = ["./valgrind/herbgrind-install/bin/valgrind", "--tool=herbgrind", "--output-sexp"] + EXTRA_ARGS + [prog]
    print("Calling `{}`".format(" ".join(command)))
    status = subprocess.call(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if status:
        print("Command failed (status {}).".format(status))
        success = False
    checkFile("{}.gh".format(prog), "{}.expected".format(prog), ignoreProps)
    if not success:
        print("Actual::")
        with open(prog + ".gh") as f: print(f.read())
        print("Expected::")
        with open(prog + ".expected") as f: print(f.read())
    return success

if __name__ == "__main__":
    ignoreProps = ["instr-addr"]

    for arg in sys.argv[1:]:
        success = test(arg, ignoreProps)
        if not success:
            sys.exit(1)
