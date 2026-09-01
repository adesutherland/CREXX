#!/bin/zsh
set -eu

if (( $# != 1 )) || [[ "$1" != baseline && "$1" != candidate ]]; then
  print -u2 -- "usage: $0 baseline|candidate"
  exit 2
fi

phase="$1"
repo_root="${0:A:h:h:h:h}"
phase_dir="${0:A:h}/$phase"
rxdas="$repo_root/cmake-build-release/bin/rxdas"
temp_dir=$(mktemp -d /tmp/nr08-library-disassembly.XXXXXX)
disassembly="$temp_dir/library.rxas"
trap 'rm -f "$disassembly"; rmdir "$temp_dir"' EXIT

"$rxdas" -o "$disassembly" "$phase_dir/artifacts/library.rxbin"

{
  print -- "module,procedure,null,endlife,copies,unlink,reference_ops"
  awk '
    function emit() {
      if (procedure != "")
        printf "%s,%s,%d,%d,%d,%d,%d\n", module, procedure, nulls,
          endlifes, copies, unlinks, references
    }
    /^\* MODULE - / {
      module=$0
      sub(/^\* MODULE - /, "", module)
      next
    }
    /\.locals=/ && $1 !~ /^\*/ {
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
  ' "$disassembly"
} > "$phase_dir/library-procedure-static-opcounts.csv"

{
  print -- "image,null,endlife,copies,unlink,reference_ops"
  awk -F, 'NR > 1 {
    nulls += $3; endlifes += $4; copies += $5; unlinks += $6; references += $7
  }
  END {
    printf "library.rxbin,%d,%d,%d,%d,%d\n", nulls, endlifes, copies,
      unlinks, references
  }' "$phase_dir/library-procedure-static-opcounts.csv"
} > "$phase_dir/library-static-opcounts.csv"

{
  print -- "command=$rxdas -o <temporary-disassembly> $phase_dir/artifacts/library.rxbin"
  shasum -a 256 "$rxdas" "$phase_dir/artifacts/library.rxbin"
} > "$phase_dir/library-analysis-state.txt"

(
  cd "$phase_dir"
  find . -type f ! -name checksums.sha256 -print | LC_ALL=C sort | \
    sed 's#^\./##' | xargs shasum -a 256
) > "$phase_dir/checksums.sha256"

print -- "PASS: retained NR-08 $phase imported-library analysis under $phase_dir"
