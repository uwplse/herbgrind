#!/usr/bin/env bash

NUM_SVS=0
NUM_STEMS=0
while read -r line
do
    if (grep -q "Making shadow value" <(echo $line)); then
        (( NUM_SVS += 1 ))
        echo -en "\e[0K\r# shadow values: $NUM_SVS  \t; # stems: $NUM_STEMS  "
    elif (grep -q "Cleaning up shadow value" <(echo $line)); then
        (( NUM_SVS -= 1 ))
        echo -en "\e[0K\r# shadow values: $NUM_SVS  \t; # stems: $NUM_STEMS  "
    elif (grep -q "Making stem node" <(echo $line)); then
        (( NUM_STEMS += 1 ))
        echo -en "\e[0K\r# shadow values: $NUM_SVS  \t; # stems: $NUM_STEMS  "
    elif (grep -q "Cleaning up stem node" <(echo $line)); then
        (( NUM_STEMS -= 1 ))
        echo -en "\e[0K\r# shadow values: $NUM_SVS  \t; # stems: $NUM_STEMS  "
    fi
done
