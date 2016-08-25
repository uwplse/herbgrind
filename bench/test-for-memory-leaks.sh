#!/bin/env bash

for f in *.out; do
    echo "testing file $f"
    ../valgrind/herbgrind-install/bin/valgrind --tool=herbgrind --print-moves ./$f &>output.log
    NUM_ALLOCED=$(grep -c "Making shadow value" $f)
    NUM_FREED=$(grep -c "Cleaning up shadow value" $f)
    if [ "$NUM_ALLOCED" -ne "$NUM_FREED" ]
    then
        echo "Found memory leak when testing against $f! see output.log for output."
        break
    fi
done
