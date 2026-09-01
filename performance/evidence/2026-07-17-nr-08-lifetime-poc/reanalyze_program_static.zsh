#!/bin/zsh

set -eu

if (( $# != 1 )) || [[ "$1" != baseline && "$1" != candidate ]]; then
  print -u2 -- "usage: $0 baseline|candidate"
  exit 2
fi

phase="$1"
phase_dir="${0:A:h}/$phase"

{
  print -- "cell,procedure,null,endlife,copies,unlink,reference_ops"
  for rxas in "$phase_dir"/artifacts/{canonical,opaque}-{noopt,opt}.rxas; do
    cell="${rxas:t:r}"
    awk -v cell="$cell" '
      function emit() {
        if (procedure != "")
          printf "%s,%s,%d,%d,%d,%d,%d\n", cell, procedure, nulls,
            endlifes, copies, unlinks, references
      }
      /^[A-Za-z_][A-Za-z0-9_]*\([^)]*\) \.locals=/ {
        emit()
        procedure=$1
        nulls=endlifes=copies=unlinks=references=0
        next
      }
      {
        operation=tolower($1)
        if (operation ~ /:$/) operation=tolower($2)
        if (operation == "null") nulls++
        else if (operation == "endlife") endlifes++
        else if (operation == "copy" || operation == "icopy" ||
                 operation == "fcopy" || operation == "scopy" ||
                 operation == "dcopy" || operation == "bcopy") copies++
        else if (operation == "unlink") unlinks++
        else if (operation == "mkref" || operation == "deref" ||
                 operation == "linkref" || operation == "setref" ||
                 operation == "refvalid" || operation == "unref") references++
      }
      END { emit() }
    ' "$rxas"
  done
} > "$phase_dir/procedure-static-opcounts.csv"

{
  print -- "cell,procedure,null,endlife,copies,unlink,reference_ops"
  awk -F, 'NR > 1 {
    key=$1 SUBSEP $2
    n[key]+=$3; e[key]+=$4; c[key]+=$5; u[key]+=$6; r[key]+=$7
  }
  END {
    for (key in n) {
      split(key, parts, SUBSEP)
      printf "%s,%s,%d,%d,%d,%d,%d\n", parts[1], parts[2],
        n[key], e[key], c[key], u[key], r[key]
    }
  }' "$phase_dir/procedure-static-opcounts.csv" | LC_ALL=C sort
} > "$phase_dir/static-opcounts.csv"

(
  cd "$phase_dir"
  find . -type f ! -name checksums.sha256 -print | LC_ALL=C sort | \
    sed 's#^\./##' | xargs shasum -a 256
) > "$phase_dir/checksums.sha256"

print -- "PASS: corrected label-aware NR-08 $phase program static analysis"
