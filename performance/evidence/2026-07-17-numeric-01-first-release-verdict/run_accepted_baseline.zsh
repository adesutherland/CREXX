#!/bin/zsh

set -euo pipefail

repo_root=${0:a:h:h:h:h}
build_dir="$repo_root/cmake-build-release"
evidence_dir="${0:a:h}/accepted-c"
raw_dir="$evidence_dir/raw"
samples_csv="$evidence_dir/samples.csv"
summary_csv="$evidence_dir/summary.csv"
image_base="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opt"
source_path="$repo_root/tests/benchmarks/rexxcps_levelb.crexx"
library_base="$build_dir/bin/library"

mkdir -p "$raw_dir"

source_sha256=$(shasum -a 256 "$source_path" | awk '{print $1}')
image_sha256=$(shasum -a 256 "$image_base.rxbin" | awk '{print $1}')
rxas_sha256=$(shasum -a 256 "$image_base.rxas" | awk '{print $1}')

print 'sequence,vm,phase,phase_sequence,source_sha256,rxas_sha256,image_sha256,native_cps,process_real_s,effective_count,pass,stdout,stderr' > "$samples_csv"
sequence=0

run_sample() {
  local vm_name=$1
  local phase=$2
  local phase_sequence=$3
  local vm_path="$build_dir/bin/$vm_name"
  local stem
  local stdout_path
  local stderr_path
  local native_cps
  local process_real_s
  local effective_count
  local pass

  (( sequence += 1 ))
  stem=$(printf '%02d-%s-%s-%d' "$sequence" "$vm_name" "$phase" "$phase_sequence")
  stdout_path="$raw_dir/$stem.stdout.txt"
  stderr_path="$raw_dir/$stem.stderr.txt"

  /usr/bin/time -p "$vm_path" "$image_base" "$library_base" > "$stdout_path" 2> "$stderr_path"

  native_cps=$(awk '/Performance:/ {print $2}' "$stdout_path")
  process_real_s=$(awk '$1 == "real" {print $2}' "$stderr_path")
  effective_count=$(sed -n 's/.* effective_count=\([0-9][0-9]*\).*/\1/p' "$stdout_path")
  if rg -q '^PASS: RexxCPS 2\.2d cREXX port$' "$stdout_path" && \
      rg -q 'REXXCPS-PROVENANCE contract=canonical-default' "$stdout_path"; then
    pass=1
  else
    pass=0
  fi

  if [[ -z "$native_cps" || -z "$process_real_s" || "$effective_count" != 100 || "$pass" != 1 ]]; then
    print -u2 "invalid accepted sample: $stem"
    exit 1
  fi

  print "$sequence,$vm_name,$phase,$phase_sequence,$source_sha256,$rxas_sha256,$image_sha256,$native_cps,$process_real_s,$effective_count,$pass,accepted-c/raw/$stem.stdout.txt,accepted-c/raw/$stem.stderr.txt" >> "$samples_csv"
  print "$stem cps=$native_cps real=$process_real_s count=$effective_count"
}

for vm_name in rxvm rxbvm; do
  run_sample "$vm_name" warmup 1
  for recorded in 1 2 3; do
    run_sample "$vm_name" recorded "$recorded"
  done
done

print 'vm,n,native_cps_median,native_cps_mean,native_cps_min,native_cps_max,process_real_median_s,process_real_mean_s,process_real_min_s,process_real_max_s' > "$summary_csv"
for vm_name in rxvm rxbvm; do
  cps_values=$(mktemp /tmp/crexx-numeric01-cps.XXXXXX)
  wall_values=$(mktemp /tmp/crexx-numeric01-wall.XXXXXX)
  awk -F, -v vm="$vm_name" '$2 == vm && $3 == "recorded" {print $8}' "$samples_csv" | sort -n > "$cps_values"
  awk -F, -v vm="$vm_name" '$2 == vm && $3 == "recorded" {print $9}' "$samples_csv" | sort -n > "$wall_values"
  cps_median=$(sed -n '2p' "$cps_values")
  cps_mean=$(awk '{sum += $1} END {printf "%.3f", sum / NR}' "$cps_values")
  cps_min=$(sed -n '1p' "$cps_values")
  cps_max=$(sed -n '3p' "$cps_values")
  wall_median=$(sed -n '2p' "$wall_values")
  wall_mean=$(awk '{sum += $1} END {printf "%.3f", sum / NR}' "$wall_values")
  wall_min=$(sed -n '1p' "$wall_values")
  wall_max=$(sed -n '3p' "$wall_values")
  print "$vm_name,3,$cps_median,$cps_mean,$cps_min,$cps_max,$wall_median,$wall_mean,$wall_min,$wall_max" >> "$summary_csv"
  rm -f -- "$cps_values" "$wall_values"
done

{
  print "source_sha256=$source_sha256"
  print "rxas_sha256=$rxas_sha256"
  print "rxbin_sha256=$image_sha256"
  print "library_sha256=$(shasum -a 256 "$build_dir/bin/library.rxbin" | awk '{print $1}')"
  print "rxvm_sha256=$(shasum -a 256 "$build_dir/bin/rxvm" | awk '{print $1}')"
  print "rxbvm_sha256=$(shasum -a 256 "$build_dir/bin/rxbvm" | awk '{print $1}')"
  print "samples_sha256=$(shasum -a 256 "$samples_csv" | awk '{print $1}')"
  print "summary_sha256=$(shasum -a 256 "$summary_csv" | awk '{print $1}')"
} > "$evidence_dir/hashes.txt"
