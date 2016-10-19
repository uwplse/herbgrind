#!/bin/env bash

source "$(dirname "$0")/paths.sh"

if ! make -C $HERBGRIND_DIR > compile.log.txt;
then
    exit $?
fi
cd $SPEC_DIR/benchspec/CPU2006/435.gromacs/run/run_base_test_amd64-m64-gcc43-nn.0003
(timeout -t 4200 -m 15000000 --confess $HERBGRIND_DIR/valgrind/herbgrind-install/bin/valgrind --tool=herbgrind --print-counts --print-moves ./gromacs_base.amd64-m64-gcc43-nn -silent -deffnm gromacs -nice 0) &> log.txt
NUM_ALLOCED=$(grep -c "Making shadow value" log.txt)
NUM_FREED=$(grep -c "Cleaning up shadow value" log.txt)
if [ "$NUM_ALLOCED" -ne "$NUM_FREED" ]
then
    (( NUM_LEAKED = NUM_ALLOCED - NUM_FREED ))
    echo "Leaked " $NUM_LEAKED " shadow values."
fi
