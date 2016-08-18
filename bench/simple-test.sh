#!/usr/bin/env bash

BDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HGPATH="${BDIR}/../valgrind/herbgrind-install/bin/valgrind"

make -C "$BDIR" > /dev/null

PASS="PASS"
FAIL="FAIL"

# ANSI color codes
BK=$'\033[1;30m' # black
RD=$'\033[1;31m' # red
GR=$'\033[1;32m' # green
YL=$'\033[1;33m' # yellow
BL=$'\033[1;34m' # blue
MG=$'\033[1;35m' # magenta
CY=$'\033[1;36m' # cyan
WT=$'\033[1;37m' # white
NC=$'\033[0m'    # no color

# only output color for ttys
if [ -t 1 ]; then
  PASS="${GR}PASS${NC}"
  FAIL="${RD}FAIL${NC}"
fi

printf "%-30s%-10s\t%-10s\n" \
       "TEST" \
       "AVG ERROR" \
       "MAX ERROR"

for t in $(ls ${BDIR}/*.out); do
  printf "%-30s" "$(basename "$t")"
  if $HGPATH --tool=herbgrind "$t" > /dev/null 2>&1; then
    err_mean=$(cat "${t}-errors.gh" \
               | grep 'bits average error' \
               | cut -d ' ' -f 1)
    if [ "$err_mean" = "" ]; then
      err_mean="N/A"
    fi
    err_max=$(cat "${t}-errors.gh" \
               | grep 'bits max error' \
               | cut -d ' ' -f 1)
    if [ "$err_max" = "" ]; then
      err_max="N/A"
    fi
    printf "%10s\t%10s\n" \
           "$err_mean" \
           "$err_max"
  else
    echo "$FAIL"
  fi
done
