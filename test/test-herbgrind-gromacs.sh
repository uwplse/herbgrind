#!/usr/bin/env bash

source "$(dirname "$0")/paths.sh"

if ! make -C $HERBGRIND_DIR > compile-log.txt;
then
    exit $?
fi
cd $SPEC_DIR/benchspec/CPU2006/435.gromacs/run/run_base_test_amd64-m64-gcc43-nn.0003
if (timeout -m 17000000 --confess $HERBGRIND_DIR/test/run-herbgrind.sh $* ./gromacs_base.amd64-m64-gcc43-nn -silent -deffnm gromacs -nice 0)
then
    echo "Success"
else
    RESULT=$?
    echo "Failure" $?
    exit $RESULT
fi
