#!/usr/bin/env bash

source "$(dirname "$0")/paths.sh"

if ! make -C $HERBGRIND_DIR > compile-log.txt;
then
    exit $?
fi

$HERBGRIND_DIR/valgrind/herbgrind-install/bin/valgrind --tool=herbgrind $* 2> >(tee log.txt)
mkdir -p $HERBGRIND_DIR/logs
FILENAME=`/usr/bin/env date -Iseconds`-`git -C $HERBGRIND_DIR rev-parse HEAD`.log
FILEPATH=$HERBGRIND_DIR/logs/$FILENAME
echo $* > $FILEPATH
echo "=============================" >> $FILEPATH
echo >> $FILEPATH
cat log.txt >> $FILEPATH
cd $HERBGRIND_DIR/logs
rm most-recent.gh
ln -s $FILENAME most-recent.gh
