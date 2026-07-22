#!/bin/zsh
set -eu

if (( $# != 1 )) || [[ "$1" != baseline && "$1" != candidate ]]; then
  print -u2 -- "usage: $0 baseline|candidate"
  exit 2
fi

phase="$1"
repo_root="${0:A:h:h:h:h}"
bundle_root="${0:A:h}"
phase_dir="$bundle_root/$phase"
release_build="$repo_root/cmake-build-release"
profile_build="$repo_root/cmake-build-profile"

mkdir -p "$phase_dir/artifacts" "$phase_dir/profiles" "$phase_dir/raw"

for source in rexxcps_levelb.crexx rexxcps_levelb_opaque.crexx; do
  cp "$repo_root/tests/benchmarks/$source" "$phase_dir/artifacts/$source"
done

for cell in canonical-noopt canonical-opt opaque-noopt opaque-opt; do
  case "$cell" in
    canonical-noopt) artifact="benchmark_rexxcps_levelb_noopt" ;;
    canonical-opt) artifact="benchmark_rexxcps_levelb_opt" ;;
    opaque-noopt) artifact="benchmark_rexxcps_levelb_opaque_noopt" ;;
    opaque-opt) artifact="benchmark_rexxcps_levelb_opaque_opt" ;;
  esac
  cp "$release_build/tests/benchmarks/$artifact.rxas" \
    "$phase_dir/artifacts/$cell.rxas"
  cp "$release_build/tests/benchmarks/$artifact.rxbin" \
    "$phase_dir/artifacts/$cell.rxbin"
done
cp "$release_build/bin/library.rxbin" "$phase_dir/artifacts/library.rxbin"

# Preserve compiler-output controls on both sides of the PoC. The reference
# fixtures exercise target and block/inline lifetime boundaries; the scoped
# fixture carries both the scalar positive case and conservative type controls.
for fixture in reference_source_inline_lifetime reference_source_block_lifetime scoped_register_reuse; do
  for mode in noopt opt; do
    flag=()
    [[ "$mode" == noopt ]] && flag=(-n)
    output="$phase_dir/artifacts/$fixture-$mode"
    "$release_build/bin/rxc" -i "$release_build/bin" "${flag[@]}" \
      -o "$output" "$repo_root/compiler/tests/rexx_src/$fixture.crexx" \
      > "$phase_dir/raw/$fixture-$mode-rxc.stdout.txt" \
      2> "$phase_dir/raw/$fixture-$mode-rxc.stderr.txt"
  done
done

git -C "$repo_root" diff -- compiler/rxcp_emit_flow.c \
  compiler/tests/CMakeLists.txt > "$phase_dir/compiler-poc.patch"

{
  print -- "captured_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print -- "phase=$phase"
  print -- "branch=$(git -C "$repo_root" branch --show-current)"
  print -- "head=$(git -C "$repo_root" rev-parse HEAD)"
  print -- "upstream=$(git -C "$repo_root" rev-parse '@{upstream}')"
  print -- "status_begin"
  git -C "$repo_root" status --short
  print -- "status_end"
  print -- "host=$(uname -srm)"
  print -- "release_build_type=$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=//p' "$release_build/CMakeCache.txt")"
  print -- "release_profiling=$(sed -n 's/^CREXX_VM_PROFILING:BOOL=//p' "$release_build/CMakeCache.txt")"
  print -- "profile_build_type=$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=//p' "$profile_build/CMakeCache.txt")"
  print -- "profile_profiling=$(sed -n 's/^CREXX_VM_PROFILING:BOOL=//p' "$profile_build/CMakeCache.txt")"
  "$profile_build/bin/rxvm" -v
  "$profile_build/bin/rxbvm" -v
} > "$phase_dir/host-build-state.txt"

(
  cd "$phase_dir"
  find artifacts -type f -print | LC_ALL=C sort | xargs shasum -a 256
  shasum -a 256 "$profile_build/bin/rxvm" "$profile_build/bin/rxbvm"
) > "$phase_dir/artifact-hashes.txt"

{
  print -- "artifact,bytes"
  for artifact in "$phase_dir"/artifacts/*; do
    print -- "${artifact:t},$(wc -c < "$artifact" | tr -d ' ')"
  done
} > "$phase_dir/artifact-sizes.csv"

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
  print -- "cell,procedure,symbol,type"
  for rxas in "$phase_dir"/artifacts/{canonical,opaque}-{noopt,opt}.rxas; do
    cell="${rxas:t:r}"
    awk -v cell="$cell" '
      /^[A-Za-z_][A-Za-z0-9_]*\([^)]*\) \.locals=/ { procedure=$1 }
      /\.meta .*="b"/ {
        line=$0
        sub(/^.*\.meta "/, "", line)
        name=line
        sub(/"=.*/, "", name)
        type=line
        sub(/^.*="b" "/, "", type)
        sub(/".*/, "", type)
        if (name !~ /\.$/ && !seen[procedure SUBSEP name]++)
          printf "%s,%s,%s,%s\n", cell, procedure, name, type
      }
    ' "$rxas"
  done
} > "$phase_dir/symbol-inventory.csv"

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

for cell in canonical-noopt canonical-opt opaque-noopt opaque-opt; do
  image="$phase_dir/artifacts/$cell.rxbin"
  for vm in rxvm rxbvm; do
    stem="$cell-$vm"
    if [[ "$cell" == canonical-* ]]; then
      "$profile_build/bin/$vm" --profile-output \
        "$phase_dir/profiles/$stem.csv" "$image" \
        "$phase_dir/artifacts/library.rxbin" -a --smoke-count 1 \
        > "$phase_dir/raw/$stem.stdout.txt" \
        2> "$phase_dir/raw/$stem.stderr.txt"
      rg -q "PASS: RexxCPS 2.2d cREXX port" \
        "$phase_dir/raw/$stem.stdout.txt"
    else
      "$profile_build/bin/$vm" --profile-output \
        "$phase_dir/profiles/$stem.csv" "$image" \
        "$phase_dir/artifacts/library.rxbin" -a A Off 1 1 \
        > "$phase_dir/raw/$stem.stdout.txt" \
        2> "$phase_dir/raw/$stem.stderr.txt"
      rg -q "PASS: RexxCPS 2.2d cREXX opaque/result-observation diagnostic" \
        "$phase_dir/raw/$stem.stdout.txt"
    fi
  done
done

{
  print -- "cell,vm,null,endlife,copy,icopy,fcopy,scopy,dcopy,bcopy,result"
  for profile in "$phase_dir"/profiles/{canonical,opaque}-{noopt,opt}-{rxvm,rxbvm}.csv; do
    stem="${profile:t:r}"
    cell="${stem%-*}"
    vm="${stem##*-}"
    awk -F, -v cell="$cell" -v vm="$vm" '
      $1 == "summary" && $2 == "result" { result=$3 }
      $1 == "instruction" && $2 == "NULL_REG" { nulls=$5 }
      $1 == "instruction" && $2 == "ENDLIFE_REG" { endlifes=$5 }
      $1 == "instruction" && $2 == "COPY_REG_REG" { copy=$5 }
      $1 == "instruction" && $2 == "ICOPY_REG_REG" { icopy=$5 }
      $1 == "instruction" && $2 == "FCOPY_REG_REG" { fcopy=$5 }
      $1 == "instruction" && $2 == "SCOPY_REG_REG" { scopy=$5 }
      $1 == "instruction" && $2 == "DCOPY_REG_REG" { dcopy=$5 }
      $1 == "instruction" && $2 == "BCOPY_REG_REG" { bcopy=$5 }
      END {
        printf "%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
          cell, vm, nulls, endlifes, copy, icopy, fcopy, scopy, dcopy,
          bcopy, result
      }
    ' "$profile"
  done
} > "$phase_dir/instruction-counts.csv"

{
  print -- "Authored source reference tokens"
  rg -n "reference|dereference|snapshot|refvalid|arg expose|procedure.*expose" \
    "$phase_dir/artifacts/rexxcps_levelb.crexx" \
    "$phase_dir/artifacts/rexxcps_levelb_opaque.crexx" || true
  print -- "Generated RXAS reference instructions or reference metadata"
  rg -n "^[[:space:]]+(mkref|deref|linkref|setref|refvalid|unref)[[:space:]]|reference" \
    "$phase_dir"/artifacts/{canonical,opaque}-{noopt,opt}.rxas || true
} > "$phase_dir/reference-scan.txt"

(
  cd "$phase_dir"
  find . -type f ! -name checksums.sha256 -print | LC_ALL=C sort | \
    sed 's#^\./##' | xargs shasum -a 256
) > "$phase_dir/checksums.sha256"

print -- "PASS: retained NR-08 $phase evidence under $phase_dir"
