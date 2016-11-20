#!/usr/bin/env bash

source "$(dirname "$0")/paths.sh"

if ! make -C $HERBGRIND_DIR > compile-log.txt;
then
    exit $?
fi
cd $TRIANGLE_DIR
time $HERBGRIND_DIR/test/run-herbgrind.sh $* ./triangle A.poly
