#!/bin/zsh

set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h:h}
candidate_build="$repo_root/cmake-build-release"
accepted_root=${ACCEPTED_ROOT:-/tmp/crexx-nr09-rebaseline-847e62f04}
accepted_build="$accepted_root/cmake-build-release"
accepted_bin="$accepted_build/bin"
candidate_bin="$candidate_build/bin"
source_path="$repo_root/tests/benchmarks/rexxcps_levelb.crexx"
images_dir="$script_dir/images"
raw_dir="$script_dir/raw"
samples_csv="$script_dir/samples.csv"
provenance_txt="$script_dir/provenance.txt"
started_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)

expected_source_sha256=2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6
expected_accepted_rxas_sha256=868fb7fdac2b387dca9ba227d0ca949431e51ea87ebb06928e874360592ee515
expected_candidate_rxas_sha256=373c33479086b4c9a8373f55a133052f5f9bbbc925cee867176400a254ccaa0b
expected_candidate_library_sha256=d1089745eb42a076e879f6c0a2e68e64865f5a801e9636e12fa27b68ce486846

fail() {
  print -u2 -- "$*"
  exit 1
}

sha256() {
  shasum -a 256 "$1" | awk '{print $1}'
}

[[ ! -e "$samples_csv" ]] || fail "refusing to overwrite $samples_csv"
[[ -d "$accepted_root/.git" || -f "$accepted_root/.git" ]] || fail "accepted worktree missing: $accepted_root"
[[ $(git -C "$accepted_root" rev-parse HEAD) == 847e62f04* ]] || fail 'accepted worktree is not 847e62f04'
[[ $(git -C "$accepted_root" status --porcelain) == '' ]] || fail 'accepted worktree is dirty'

for build_dir in "$accepted_build" "$candidate_build"; do
  rg -q '^CMAKE_BUILD_TYPE:STRING=Release$' "$build_dir/CMakeCache.txt" || fail "$build_dir is not Release"
  rg -q '^CREXX_VM_PROFILING:BOOL=OFF$' "$build_dir/CMakeCache.txt" || fail "$build_dir has profiling enabled"
done

for bin_dir in "$accepted_bin" "$candidate_bin"; do
  for artifact in rxc rxas rxlink rxvm rxbvm library.rxbin rxcexits.rxbin; do
    [[ -f "$bin_dir/$artifact" ]] || fail "missing $bin_dir/$artifact"
  done
done

[[ $(sha256 "$source_path") == "$expected_source_sha256" ]] || fail 'canonical source hash mismatch'
[[ $(sha256 "$candidate_bin/library.rxbin") == "$expected_candidate_library_sha256" ]] || fail 'candidate library drifted from frozen batch'

mkdir -p "$images_dir" "$raw_dir"

accepted_rxas="$images_dir/cell-a.rxas"
accepted_image="$images_dir/cell-a"
candidate_rxas="$images_dir/cell-c.rxas"
candidate_image="$images_dir/cell-c"

"$accepted_bin/rxc" -i "$accepted_bin" -o "$accepted_rxas" "$source_path"
"$accepted_bin/rxas" -o "$accepted_image" "$accepted_rxas"
"$candidate_bin/rxc" -i "$candidate_bin" -o "$candidate_rxas" "$source_path"
"$candidate_bin/rxas" -o "$candidate_image" "$candidate_rxas"

[[ $(sha256 "$accepted_rxas") == "$expected_accepted_rxas_sha256" ]] || fail 'accepted RXAS does not reproduce the accepted hash'
[[ $(sha256 "$candidate_rxas") == "$expected_candidate_rxas_sha256" ]] || fail 'candidate RXAS does not reproduce the frozen batch hash'
cmp -s "$candidate_rxas" "$script_dir/../candidate-rexxcps-opt.rxas" || fail 'candidate RXAS differs from frozen first-verdict RXAS'

{
  print "started_utc=$started_utc"
  print "repo_root=$repo_root"
  print "candidate_branch=$(git -C "$repo_root" branch --show-current)"
  print "candidate_head=$(git -C "$repo_root" rev-parse HEAD)"
  print "accepted_root=$accepted_root"
  print "accepted_head=$(git -C "$accepted_root" rev-parse HEAD)"
  print "source_sha256=$(sha256 "$source_path")"
  print "accepted_rxas_sha256=$(sha256 "$accepted_rxas")"
  print "accepted_rxbin_sha256=$(sha256 "$accepted_image.rxbin")"
  print "accepted_library_sha256=$(sha256 "$accepted_bin/library.rxbin")"
  print "accepted_rxvm_sha256=$(sha256 "$accepted_bin/rxvm")"
  print "accepted_rxbvm_sha256=$(sha256 "$accepted_bin/rxbvm")"
  print "candidate_rxas_sha256=$(sha256 "$candidate_rxas")"
  print "candidate_rxbin_sha256=$(sha256 "$candidate_image.rxbin")"
  print "candidate_library_sha256=$(sha256 "$candidate_bin/library.rxbin")"
  print "candidate_rxvm_sha256=$(sha256 "$candidate_bin/rxvm")"
  print "candidate_rxbvm_sha256=$(sha256 "$candidate_bin/rxbvm")"
  print 'schedule=12 recorded rounds; all six A/B/C permutations twice; reverse-symmetric; VM order alternates by round'
  print 'cell_A=accepted VM + accepted RXBIN + accepted library'
  print 'cell_B=candidate VM + accepted RXBIN + accepted library'
  print 'cell_C=candidate VM + candidate RXBIN + candidate library'
  print "uname=$(uname -a)"
  print "cpu=$(sysctl -n machdep.cpu.brand_string 2>/dev/null || true)"
  print "logical_cpus=$(sysctl -n hw.logicalcpu 2>/dev/null || true)"
  sw_vers
  uptime
  pmset -g batt 2>/dev/null || true
  cmake --version | head -n 1
  ninja --version | awk '{print "ninja " $0}'
  clang --version | head -n 1
  print 'accepted_build_options:'
  rg '^(CMAKE_BUILD_TYPE|CMAKE_C_FLAGS_RELEASE|CREXX_VM_PROFILING|CMAKE_GENERATOR):' "$accepted_build/CMakeCache.txt"
  print 'candidate_build_options:'
  rg '^(CMAKE_BUILD_TYPE|CMAKE_C_FLAGS_RELEASE|CREXX_VM_PROFILING|CMAKE_GENERATOR):' "$candidate_build/CMakeCache.txt"
  print 'candidate_dirty_worktree:'
  git -C "$repo_root" status --short
} > "$provenance_txt"

print 'sequence,vm_mode,phase,round,position,cell,vm_binary,image,library,native_cps,process_real_s,effective_count,pass,started_utc,ended_utc,stdout,stderr' > "$samples_csv"
sequence=0

run_sample() {
  local vm_mode=$1
  local phase=$2
  local round=$3
  local position=$4
  local cell=$5
  local vm_path
  local image_base
  local library_base
  local stem
  local stdout_path
  local stderr_path
  local native_cps
  local process_real_s
  local effective_count
  local pass
  local sample_started
  local sample_ended

  case "$cell" in
    A)
      vm_path="$accepted_bin/$vm_mode"
      image_base="$accepted_image"
      library_base="$accepted_bin/library"
      ;;
    B)
      vm_path="$candidate_bin/$vm_mode"
      image_base="$accepted_image"
      library_base="$accepted_bin/library"
      ;;
    C)
      vm_path="$candidate_bin/$vm_mode"
      image_base="$candidate_image"
      library_base="$candidate_bin/library"
      ;;
    *) fail "unknown cell: $cell" ;;
  esac

  (( sequence += 1 ))
  stem=$(printf '%03d-%s-%s-r%02d-p%d-cell%s' "$sequence" "$vm_mode" "$phase" "$round" "$position" "$cell")
  stdout_path="$raw_dir/$stem.stdout.txt"
  stderr_path="$raw_dir/$stem.stderr.txt"
  sample_started=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  /usr/bin/time -p "$vm_path" "$image_base" "$library_base" > "$stdout_path" 2> "$stderr_path"
  sample_ended=$(date -u +%Y-%m-%dT%H:%M:%SZ)

  native_cps=$(awk '/Performance:/ {print $2}' "$stdout_path")
  process_real_s=$(awk '$1 == "real" {print $2}' "$stderr_path")
  effective_count=$(sed -n 's/.* effective_count=\([0-9][0-9]*\).*/\1/p' "$stdout_path")
  pass=0
  if rg -q '^PASS: RexxCPS 2\.2d cREXX port$' "$stdout_path" && \
     rg -q 'REXXCPS-PROVENANCE contract=canonical-default argv_count=0' "$stdout_path"; then
    pass=1
  fi
  [[ -n "$native_cps" && -n "$process_real_s" && "$effective_count" == 100 && "$pass" == 1 ]] || fail "invalid sample: $stem"

  print "$sequence,$vm_mode,$phase,$round,$position,$cell,$vm_path,$image_base.rxbin,$library_base.rxbin,$native_cps,$process_real_s,$effective_count,$pass,$sample_started,$sample_ended,raw/$stem.stdout.txt,raw/$stem.stderr.txt" >> "$samples_csv"
  print "$stem cps=$native_cps real=$process_real_s"
}

# Every product/VM cell receives one full canonical warmup.
for vm_mode in rxvm rxbvm; do
  if [[ "$vm_mode" == rxvm ]]; then
    warmup_order=ABC
  else
    warmup_order=CBA
  fi
  for position in 1 2 3; do
    run_sample "$vm_mode" warmup 0 "$position" "$warmup_order[$position]"
  done
done

# The second six rounds mirror the first six, cancelling monotonic session drift.
permutations=(ABC BCA CAB CBA ACB BAC BAC ACB CBA CAB BCA ABC)
for round in {1..12}; do
  order=$permutations[$round]
  if (( round % 2 == 1 )); then
    vm_order=(rxvm rxbvm)
  else
    vm_order=(rxbvm rxvm)
  fi
  for vm_mode in $vm_order; do
    for position in 1 2 3; do
      run_sample "$vm_mode" recorded "$round" "$position" "$order[$position]"
    done
  done
done

print "ended_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)" >> "$provenance_txt"
python3 "$script_dir/summarize_three_cell.py" "$samples_csv" "$script_dir"

{
  shasum -a 256 "$source_path" "$accepted_rxas" "$accepted_image.rxbin" \
    "$accepted_bin/library.rxbin" "$accepted_bin/rxvm" "$accepted_bin/rxbvm" \
    "$candidate_rxas" "$candidate_image.rxbin" "$candidate_bin/library.rxbin" \
    "$candidate_bin/rxvm" "$candidate_bin/rxbvm" "$samples_csv" \
    "$script_dir/cell-summary.csv" "$script_dir/paired-deltas.csv" \
    "$script_dir/paired-summary.csv" "$script_dir/position-summary.csv" \
    "$script_dir/historical-drift.csv" "$provenance_txt"
} > "$script_dir/sha256sums.txt"

print
print 'Cell summary:'
sed -n '1,7p' "$script_dir/cell-summary.csv"
print
print 'Paired summary:'
sed -n '1,7p' "$script_dir/paired-summary.csv"
