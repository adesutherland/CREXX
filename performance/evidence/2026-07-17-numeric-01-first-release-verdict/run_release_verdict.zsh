#!/bin/zsh

set -euo pipefail

if (( $# != 1 )); then
  print -u2 "usage: $0 CANDIDATE_DIR"
  exit 2
fi

repo_root=${0:a:h:h:h:h}
candidate_dir=${1:a}
build_dir="$repo_root/cmake-build-release"
evidence_dir=${0:a:h}
raw_dir="$evidence_dir/raw"
samples_csv="$evidence_dir/samples.csv"

mkdir -p "$raw_dir"

typeset -A image_base
image_base[A]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opt"
image_base[B]="$candidate_dir/rexxcps_B_canonical_opt"
image_base[C]="$candidate_dir/rexxcps_C_canonical_opt"

for candidate in A B C; do
  if [[ ! -f "${image_base[$candidate]}.rxbin" ]]; then
    print -u2 "missing image: ${image_base[$candidate]}.rxbin"
    exit 2
  fi
done

print 'sequence,vm,round,position,candidate,image_sha256,native_cps,process_real_s,effective_count,pass,stdout,stderr' > "$samples_csv"

sequence=0

run_sample() {
  local vm_name=$1
  local round=$2
  local position=$3
  local candidate=$4
  local vm_path="$build_dir/bin/$vm_name"
  local library_base="$build_dir/bin/library"
  local stem
  local stdout_path
  local stderr_path
  local image_sha256
  local native_cps
  local process_real_s
  local effective_count
  local pass

  (( sequence += 1 ))
  stem=$(printf '%02d-%s-r%d-p%d-%s' "$sequence" "$vm_name" "$round" "$position" "$candidate")
  stdout_path="$raw_dir/$stem.stdout.txt"
  stderr_path="$raw_dir/$stem.stderr.txt"

  /usr/bin/time -p "$vm_path" "${image_base[$candidate]}" "$library_base" > "$stdout_path" 2> "$stderr_path"

  image_sha256=$(shasum -a 256 "${image_base[$candidate]}.rxbin" | awk '{print $1}')
  native_cps=$(awk '/Performance:/ {print $2}' "$stdout_path")
  process_real_s=$(awk '$1 == "real" {print $2}' "$stderr_path")
  effective_count=$(sed -n 's/.* effective_count=\([0-9][0-9]*\).*/\1/p' "$stdout_path")
  if rg -q '^PASS: RexxCPS 2\.2c cREXX' "$stdout_path"; then
    pass=1
  else
    pass=0
  fi

  if [[ -z "$native_cps" || -z "$process_real_s" || -z "$effective_count" || "$pass" != 1 ]]; then
    print -u2 "invalid sample: $stem"
    exit 1
  fi

  print "$sequence,$vm_name,$round,$position,$candidate,$image_sha256,$native_cps,$process_real_s,$effective_count,$pass,raw/$stem.stdout.txt,raw/$stem.stderr.txt" >> "$samples_csv"
  print "$stem cps=$native_cps real=$process_real_s count=$effective_count"
}

typeset -a rxvm_orders
rxvm_orders=("A B C" "B C A" "C A B")

typeset -a rxbvm_orders
rxbvm_orders=("C B A" "B A C" "A C B")

round=0
for order in $rxvm_orders; do
  (( round += 1 ))
  position=0
  for candidate in ${(z)order}; do
    (( position += 1 ))
    run_sample rxvm "$round" "$position" "$candidate"
  done
done

round=0
for order in $rxbvm_orders; do
  (( round += 1 ))
  position=0
  for candidate in ${(z)order}; do
    (( position += 1 ))
    run_sample rxbvm "$round" "$position" "$candidate"
  done
done
