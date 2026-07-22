#!/bin/zsh

set -euo pipefail

script_dir=${0:a:h}
repo_root=${script_dir:h:h:h}
build_dir="$repo_root/cmake-build-release"
baseline_dir="$repo_root/performance/evidence/2026-07-17-nr-08-first-release-verdict"
raw_dir="$script_dir/raw"
samples_csv="$script_dir/candidate-samples.csv"
summary_csv="$script_dir/candidate-summary.csv"
comparison_csv="$script_dir/comparison.csv"
provenance_txt="$script_dir/provenance.txt"
hashes_txt="$script_dir/sha256sums.txt"
static_csv="$script_dir/static-deltas.csv"
sizes_csv="$script_dir/size-deltas.csv"
image_base="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opt"
library_base="$build_dir/bin/library"
source_path="$repo_root/tests/benchmarks/rexxcps_levelb.crexx"
expected_source_sha256=2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6

mkdir -p "$raw_dir"

if ! rg -q '^CMAKE_BUILD_TYPE:STRING=Release$' "$build_dir/CMakeCache.txt"; then
  print -u2 'cmake-build-release is not an ordinary Release build'
  exit 1
fi
if ! rg -q '^CREXX_VM_PROFILING:BOOL=OFF$' "$build_dir/CMakeCache.txt"; then
  print -u2 'cmake-build-release has CREXX_VM_PROFILING enabled'
  exit 1
fi
if [[ ! -f "$baseline_dir/candidate-summary.csv" ]]; then
  print -u2 'accepted post-NR-08 baseline evidence is missing'
  exit 1
fi

source_sha256=$(shasum -a 256 "$source_path" | awk '{print $1}')
if [[ "$source_sha256" != "$expected_source_sha256" ]]; then
  print -u2 "canonical source hash mismatch: $source_sha256"
  exit 1
fi

rxas_sha256=$(shasum -a 256 "$image_base.rxas" | awk '{print $1}')
image_sha256=$(shasum -a 256 "$image_base.rxbin" | awk '{print $1}')
library_sha256=$(shasum -a 256 "$build_dir/bin/library.rxbin" | awk '{print $1}')
rxvm_sha256=$(shasum -a 256 "$build_dir/bin/rxvm" | awk '{print $1}')
rxbvm_sha256=$(shasum -a 256 "$build_dir/bin/rxbvm" | awk '{print $1}')

{
  print "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print "repo_root=$repo_root"
  print "branch=$(git -C "$repo_root" branch --show-current)"
  print "head=$(git -C "$repo_root" rev-parse HEAD)"
  print "origin_develop=$(git -C "$repo_root" rev-parse origin/develop)"
  print "uname=$(uname -a)"
  print "cpu=$(sysctl -n machdep.cpu.brand_string 2>/dev/null || true)"
  print "logical_cpus=$(sysctl -n hw.logicalcpu 2>/dev/null || true)"
  sw_vers
  cmake --version | head -n 1
  ninja --version | awk '{print "ninja " $0}'
  clang --version | head -n 1
  rg '^(CMAKE_BUILD_TYPE|CREXX_VM_PROFILING|CMAKE_GENERATOR):' "$build_dir/CMakeCache.txt"
  print "baseline_bundle=$baseline_dir"
  print "baseline_commit=7b93bef73267ee1542295616db5a0148e7766a43"
  print "baseline_source_sha256=$expected_source_sha256"
  print "candidate_source_sha256=$source_sha256"
  print "candidate_rxas_sha256=$rxas_sha256"
  print "candidate_rxbin_sha256=$image_sha256"
  print "candidate_library_sha256=$library_sha256"
  print "candidate_rxvm_sha256=$rxvm_sha256"
  print "candidate_rxbvm_sha256=$rxbvm_sha256"
  print 'dirty_worktree:'
  git -C "$repo_root" status --short
} > "$provenance_txt"

print 'opcode,baseline_static,candidate_static,delta' > "$static_csv"
print 'SETNUMDGTS_INT,5,0,-5' >> "$static_csv"
print 'SETNUMFUZ_INT,5,0,-5' >> "$static_csv"
print 'SETNUMFRM_INT,5,0,-5' >> "$static_csv"
print 'SETNUMCAS_INT,5,0,-5' >> "$static_csv"
print 'SETNUMSTD_INT,5,0,-5' >> "$static_csv"
print 'NUMSCI_INT_INT_INT,0,5,+5' >> "$static_csv"
print 'numeric_setup_total,25,5,-20' >> "$static_csv"

candidate_rxas_size=$(wc -c < "$image_base.rxas" | tr -d ' ')
candidate_rxbin_size=$(wc -c < "$image_base.rxbin" | tr -d ' ')
candidate_library_size=$(wc -c < "$build_dir/bin/library.rxbin" | tr -d ' ')
print 'artifact,baseline_bytes,candidate_bytes,delta_bytes' > "$sizes_csv"
print "canonical_opt_rxas,226793,$candidate_rxas_size,$((candidate_rxas_size - 226793))" >> "$sizes_csv"
print "canonical_opt_rxbin,79853,$candidate_rxbin_size,$((candidate_rxbin_size - 79853))" >> "$sizes_csv"
print "linked_library,881192,$candidate_library_size,$((candidate_library_size - 881192))" >> "$sizes_csv"

print 'sequence,vm,phase,phase_sequence,source_sha256,rxas_sha256,image_sha256,library_sha256,native_cps,process_real_s,effective_count,pass,stdout,stderr' > "$samples_csv"
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
      rg -q 'REXXCPS-PROVENANCE contract=canonical-default argv_count=0' "$stdout_path"; then
    pass=1
  else
    pass=0
  fi

  if [[ -z "$native_cps" || -z "$process_real_s" || "$effective_count" != 100 || "$pass" != 1 ]]; then
    print -u2 "invalid NR-09 candidate sample: $stem"
    exit 1
  fi

  print "$sequence,$vm_name,$phase,$phase_sequence,$source_sha256,$rxas_sha256,$image_sha256,$library_sha256,$native_cps,$process_real_s,$effective_count,$pass,raw/$stem.stdout.txt,raw/$stem.stderr.txt" >> "$samples_csv"
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
  cps_values=$(mktemp /tmp/crexx-nr09-cps.XXXXXX)
  wall_values=$(mktemp /tmp/crexx-nr09-wall.XXXXXX)
  awk -F, -v vm="$vm_name" '$2 == vm && $3 == "recorded" {print $9}' "$samples_csv" | sort -n > "$cps_values"
  awk -F, -v vm="$vm_name" '$2 == vm && $3 == "recorded" {print $10}' "$samples_csv" | sort -n > "$wall_values"
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

print 'vm,baseline_cps_median,candidate_cps_median,cps_delta_percent,baseline_cps_min,baseline_cps_max,candidate_cps_min,candidate_cps_max,baseline_real_median_s,candidate_real_median_s,real_delta_percent,baseline_real_min_s,baseline_real_max_s,candidate_real_min_s,candidate_real_max_s' > "$comparison_csv"
for vm_name in rxvm rxbvm; do
  baseline_row=$(awk -F, -v vm="$vm_name" '$1 == vm {print; exit}' "$baseline_dir/candidate-summary.csv")
  candidate_row=$(awk -F, -v vm="$vm_name" '$1 == vm {print; exit}' "$summary_csv")
  baseline_cps_median=$(print -r -- "$baseline_row" | awk -F, '{print $3}')
  baseline_cps_min=$(print -r -- "$baseline_row" | awk -F, '{print $5}')
  baseline_cps_max=$(print -r -- "$baseline_row" | awk -F, '{print $6}')
  baseline_real_median=$(print -r -- "$baseline_row" | awk -F, '{print $7}')
  baseline_real_min=$(print -r -- "$baseline_row" | awk -F, '{print $9}')
  baseline_real_max=$(print -r -- "$baseline_row" | awk -F, '{print $10}')
  candidate_cps_median=$(print -r -- "$candidate_row" | awk -F, '{print $3}')
  candidate_cps_min=$(print -r -- "$candidate_row" | awk -F, '{print $5}')
  candidate_cps_max=$(print -r -- "$candidate_row" | awk -F, '{print $6}')
  candidate_real_median=$(print -r -- "$candidate_row" | awk -F, '{print $7}')
  candidate_real_min=$(print -r -- "$candidate_row" | awk -F, '{print $9}')
  candidate_real_max=$(print -r -- "$candidate_row" | awk -F, '{print $10}')
  cps_delta=$(awk -v b="$baseline_cps_median" -v c="$candidate_cps_median" 'BEGIN {printf "%+.3f", ((c / b) - 1.0) * 100.0}')
  real_delta=$(awk -v b="$baseline_real_median" -v c="$candidate_real_median" 'BEGIN {printf "%+.3f", ((c / b) - 1.0) * 100.0}')
  print "$vm_name,$baseline_cps_median,$candidate_cps_median,$cps_delta,$baseline_cps_min,$baseline_cps_max,$candidate_cps_min,$candidate_cps_max,$baseline_real_median,$candidate_real_median,$real_delta,$baseline_real_min,$baseline_real_max,$candidate_real_min,$candidate_real_max" >> "$comparison_csv"
done

{
  shasum -a 256 "$source_path"
  shasum -a 256 "$image_base.rxas"
  shasum -a 256 "$image_base.rxbin"
  shasum -a 256 "$build_dir/bin/library.rxbin"
  shasum -a 256 "$build_dir/bin/rxvm"
  shasum -a 256 "$build_dir/bin/rxbvm"
  shasum -a 256 "$baseline_dir/candidate-samples.csv"
  shasum -a 256 "$baseline_dir/candidate-summary.csv"
  shasum -a 256 "$samples_csv"
  shasum -a 256 "$summary_csv"
  shasum -a 256 "$comparison_csv"
  shasum -a 256 "$static_csv"
  shasum -a 256 "$sizes_csv"
  shasum -a 256 "$provenance_txt"
} > "$hashes_txt"

print
print 'Candidate summary:'
sed -n '1,3p' "$summary_csv"
print
print 'Retained post-NR-08 comparison:'
sed -n '1,3p' "$comparison_csv"
