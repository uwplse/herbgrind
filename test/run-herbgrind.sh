#!/usr/bin/env bash

source "$(dirname "$0")/paths.sh"

if ! make -C $HERBGRIND_DIR > compile-log.txt;
then
    exit $?
fi

$HERBGRIND_DIR/valgrind/herbgrind-install/bin/valgrind --tool=herbgrind --outfile=output.gh $*
mkdir -p $HERBGRIND_DIR/logs
FILENAME=$HERBGRIND_DIR/logs/`/usr/bin/date -Iseconds`-`git rev-parse HEAD`.gh
echo $* > $FILENAME
echo "=============================" >> $FILENAME
echo >> $FILENAME
cat output.gh >> $FILENAME
