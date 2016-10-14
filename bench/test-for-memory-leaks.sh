#!/bin/env bash

for f in *.out; do
    echo "testing file $f"
    ../valgrind/herbgrind-install/bin/valgrind --tool=herbgrind --print-moves ./$f &>output.log
    NUM_SV_ALLOCED=$(grep -c "Making shadow value" output.log)
    NUM_SV_FREED=$(grep -c "Cleaning up shadow value" output.log)
    if [ "$NUM_SV_ALLOCED" -ne "$NUM_SV_FREED" ]
    then
        echo "Found shadow value leak when testing against $f! see output.log for output."
        exit 1
    fi
    NUM_STEM_ALLOCED=$(grep -c "Making stem node" output.log)
    NUM_STEM_FREED=$(grep -c "Cleaning up stem node" output.log)
    if [ "$NUM_STEM_ALLOCED" -ne "$NUM_STEM_FREED" ]
    then
        echo "Found stem leak when testing against $f! see output.log for output."
        exit 1
    fi
done
