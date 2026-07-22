#!/bin/zsh

set -euo pipefail

phase=${1:-}
script_dir=${0:a:h}
repo_root=${script_dir:h:h:h}
release_dir="$repo_root/cmake-build-release"
profile_dir="$repo_root/cmake-build-profile"
accepted_dir="$repo_root/performance/evidence/2026-07-17-nr-09-numctx-first-release-verdict"
old_product_dir="$repo_root/performance/evidence/2026-07-17-nr-08-lifetime-poc/candidate/artifacts"
old_verdict_dir="$repo_root/performance/evidence/2026-07-17-nr-08-first-release-verdict"
static_dir="$script_dir/static"
raw_dir="$script_dir/raw"
profiles_dir="$script_dir/profiles"
profile_raw_dir="$script_dir/profile-raw"
expected_baseline_rxc_sha256=b65c0ceb433c97f4c67be8c1159c1192fc3e0372892ecad0b56ebee88b3e55af
source_path="$repo_root/tests/benchmarks/rexxcps_levelb.crexx"
expected_source_sha256=2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6

workloads=(
  awfy_bounce
  awfy_list
  awfy_mandelbrot
  awfy_permute
  awfy_richards
  awfy_sieve
  awfy_storage
  awfy_towers
  base64_roundtrip
  json_parser
  rexxcps_levelb
)

compile_portfolio() {
  local compiler=$1
  local bin_dir=$2
  local output_dir=$3
  local workload
  local mode
  local flags
  mkdir -p "$output_dir"
  for workload in $workloads; do
    for mode in noopt opt; do
      flags=()
      if [[ "$mode" == noopt ]]; then flags=(-n); fi
      "$compiler" -i "$bin_dir" $flags \
        -o "$output_dir/$workload-$mode.rxas" \
        "$repo_root/tests/benchmarks/$workload.crexx"
    done
  done
}

assert_release_tree() {
  rg -q '^CMAKE_BUILD_TYPE:STRING=Release$' "$release_dir/CMakeCache.txt"
  rg -q '^CREXX_VM_PROFILING:BOOL=OFF$' "$release_dir/CMakeCache.txt"
}

capture_baseline() {
  local current_hash
  assert_release_tree
  current_hash=$(shasum -a 256 "$release_dir/bin/rxc" | awk '{print $1}')
  if [[ "$current_hash" != "$expected_baseline_rxc_sha256" ]]; then
    print -u2 "pre-batch Release rxc hash mismatch: $current_hash"
    exit 1
  fi
  mkdir -p "$static_dir/baseline"
  compile_portfolio "$release_dir/bin/rxc" "$release_dir/bin" \
                    "$static_dir/baseline"
  {
    print "captured_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    print "head=$(git -C "$repo_root" rev-parse HEAD)"
    print "accepted_timing_bundle=$accepted_dir"
    shasum -a 256 "$release_dir/bin/rxc"
    "$release_dir/bin/rxc" -v
    find "$static_dir/baseline" -name '*.rxas' -type f -print | \
      sort | xargs shasum -a 256
  } > "$script_dir/baseline-capture.txt"
}

write_static_deltas() {
  local workload
  local mode
  local base
  local candidate
  local baseline_count
  local candidate_count
  local baseline_bytes
  local candidate_bytes

  print 'image,baseline_instructions,candidate_instructions,delta,baseline_rxas_bytes,candidate_rxas_bytes,delta_bytes' \
    > "$script_dir/static-image-deltas.csv"
  for workload in $workloads; do
    for mode in noopt opt; do
      base="$static_dir/baseline/$workload-$mode.rxas"
      candidate="$static_dir/candidate/$workload-$mode.rxas"
      baseline_count=$(awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {n++} END {print n+0}' "$base")
      candidate_count=$(awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {n++} END {print n+0}' "$candidate")
      baseline_bytes=$(wc -c < "$base" | tr -d ' ')
      candidate_bytes=$(wc -c < "$candidate" | tr -d ' ')
      print "$workload-$mode,$baseline_count,$candidate_count,$((candidate_count-baseline_count)),$baseline_bytes,$candidate_bytes,$((candidate_bytes-baseline_bytes))" \
        >> "$script_dir/static-image-deltas.csv"
    done
  done

  awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {print $1}' "$static_dir"/baseline/*.rxas | \
    sort | uniq -c | awk '{print $2 "," $1}' > "$static_dir/baseline-mnemonics.csv"
  awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {print $1}' "$static_dir"/candidate/*.rxas | \
    sort | uniq -c | awk '{print $2 "," $1}' > "$static_dir/candidate-mnemonics.csv"
  local unsorted_deltas
  unsorted_deltas=$(mktemp /tmp/crexx-nr09-static-deltas.XXXXXX)
  awk -F, '
    FNR==NR {base[$1]=$2; names[$1]=1; next}
    {candidate[$1]=$2; names[$1]=1}
    END {
      for (name in names) printf "%s,%d,%d,%+d\n", name, base[name]+0, candidate[name]+0, (candidate[name]+0)-(base[name]+0)
    }' "$static_dir/baseline-mnemonics.csv" "$static_dir/candidate-mnemonics.csv" | \
    sort > "$unsorted_deltas"
  print 'mnemonic,baseline_static,candidate_static,delta' \
    > "$script_dir/static-mnemonic-deltas.csv"
  cat "$unsorted_deltas" >> "$script_dir/static-mnemonic-deltas.csv"
  rm -f -- "$unsorted_deltas"
}

run_sample() {
  local vm_name=$1
  local phase_name=$2
  local phase_sequence=$3
  local sequence=$4
  local image_base=$5
  local stem
  local stdout_path
  local stderr_path
  local native_cps
  local process_real_s
  local effective_count
  local pass
  stem=$(printf '%02d-%s-%s-%d' "$sequence" "$vm_name" "$phase_name" "$phase_sequence")
  stdout_path="$raw_dir/$stem.stdout.txt"
  stderr_path="$raw_dir/$stem.stderr.txt"
  /usr/bin/time -p "$release_dir/bin/$vm_name" "$image_base" \
    "$release_dir/bin/library" > "$stdout_path" 2> "$stderr_path"
  native_cps=$(awk '/Performance:/ {print $2}' "$stdout_path")
  process_real_s=$(awk '$1 == "real" {print $2}' "$stderr_path")
  effective_count=$(sed -n 's/.* effective_count=\([0-9][0-9]*\).*/\1/p' "$stdout_path")
  pass=0
  if rg -q '^PASS: RexxCPS 2\.2d cREXX port$' "$stdout_path" && \
     rg -q 'REXXCPS-PROVENANCE contract=canonical-default argv_count=0' "$stdout_path"; then
    pass=1
  fi
  if [[ -z "$native_cps" || -z "$process_real_s" || "$effective_count" != 100 || "$pass" != 1 ]]; then
    print -u2 "invalid candidate sample: $stem"
    exit 1
  fi
  print "$sequence,$vm_name,$phase_name,$phase_sequence,$native_cps,$process_real_s,$effective_count,$pass,raw/$stem.stdout.txt,raw/$stem.stderr.txt" \
    >> "$script_dir/candidate-samples.csv"
  print "$stem cps=$native_cps real=$process_real_s"
}

write_timing_summary() {
  local vm_name
  local cps_values
  local wall_values
  local baseline_row
  local candidate_row
  print 'vm,n,native_cps_median,native_cps_mean,native_cps_min,native_cps_max,process_real_median_s,process_real_mean_s,process_real_min_s,process_real_max_s' \
    > "$script_dir/candidate-summary.csv"
  for vm_name in rxvm rxbvm; do
    cps_values=$(mktemp /tmp/crexx-nr09-batch-cps.XXXXXX)
    wall_values=$(mktemp /tmp/crexx-nr09-batch-wall.XXXXXX)
    awk -F, -v vm="$vm_name" '$2==vm && $3=="recorded" {print $5}' "$script_dir/candidate-samples.csv" | sort -n > "$cps_values"
    awk -F, -v vm="$vm_name" '$2==vm && $3=="recorded" {print $6}' "$script_dir/candidate-samples.csv" | sort -n > "$wall_values"
    print "$vm_name,3,$(sed -n '2p' "$cps_values"),$(awk '{s+=$1} END {printf "%.3f",s/NR}' "$cps_values"),$(sed -n '1p' "$cps_values"),$(sed -n '3p' "$cps_values"),$(sed -n '2p' "$wall_values"),$(awk '{s+=$1} END {printf "%.3f",s/NR}' "$wall_values"),$(sed -n '1p' "$wall_values"),$(sed -n '3p' "$wall_values")" \
      >> "$script_dir/candidate-summary.csv"
    rm -f -- "$cps_values" "$wall_values"
  done

  print 'vm,baseline_cps_median,candidate_cps_median,cps_delta_percent,baseline_cps_min,baseline_cps_max,candidate_cps_min,candidate_cps_max,baseline_real_median_s,candidate_real_median_s,real_delta_percent,baseline_real_min_s,baseline_real_max_s,candidate_real_min_s,candidate_real_max_s' \
    > "$script_dir/comparison.csv"
  for vm_name in rxvm rxbvm; do
    baseline_row=$(awk -F, -v vm="$vm_name" '$1==vm {print;exit}' "$accepted_dir/candidate-summary.csv")
    candidate_row=$(awk -F, -v vm="$vm_name" '$1==vm {print;exit}' "$script_dir/candidate-summary.csv")
    print -r -- "$baseline_row|$candidate_row" | awk -F'[|,]' '{
      printf "%s,%s,%s,%+.3f,%s,%s,%s,%s,%s,%s,%+.3f,%s,%s,%s,%s\n",
        $1,$3,$13,(($13/$3)-1)*100,$5,$6,$15,$16,$7,$17,(($17/$7)-1)*100,$9,$10,$19,$20
    }' >> "$script_dir/comparison.csv"
  done
}

capture_candidate() {
  local source_sha256
  local image_base
  local sequence
  local vm_name
  local recorded
  assert_release_tree
  source_sha256=$(shasum -a 256 "$source_path" | awk '{print $1}')
  [[ "$source_sha256" == "$expected_source_sha256" ]]
  [[ -f "$script_dir/baseline-capture.txt" ]]
  mkdir -p "$static_dir/candidate" "$raw_dir"
  compile_portfolio "$release_dir/bin/rxc" "$release_dir/bin" \
                    "$static_dir/candidate"
  write_static_deltas
  mkdir -p "$static_dir/candidate-rxbin"
  for candidate_rxas in "$static_dir/candidate"/*.rxas; do
    "$release_dir/bin/rxas" -o \
      "$static_dir/candidate-rxbin/${candidate_rxas:t:r}.rxbin" \
      "$candidate_rxas"
  done
  image_base="$script_dir/candidate-rexxcps-opt"
  cp "$static_dir/candidate/rexxcps_levelb-opt.rxas" "$image_base.rxas"
  "$release_dir/bin/rxas" -o "$image_base.rxbin" "$image_base.rxas"

  print 'sequence,vm,phase,phase_sequence,native_cps,process_real_s,effective_count,pass,stdout,stderr' \
    > "$script_dir/candidate-samples.csv"
  sequence=0
  for vm_name in rxvm rxbvm; do
    (( sequence += 1 ))
    run_sample "$vm_name" warmup 1 "$sequence" "$image_base"
    for recorded in 1 2 3; do
      (( sequence += 1 ))
      run_sample "$vm_name" recorded "$recorded" "$sequence" "$image_base"
    done
  done
  write_timing_summary

  {
    print "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    print "branch=$(git -C "$repo_root" branch --show-current)"
    print "head=$(git -C "$repo_root" rev-parse HEAD)"
    print "source_sha256=$source_sha256"
    print "accepted_baseline_bundle=$accepted_dir"
    print "accepted_baseline_commit=847e62f04"
    print "arbitrary_operand_prerequisite_commit=32bf7e76f"
    uname -a
    sw_vers
    rg '^(CMAKE_BUILD_TYPE|CREXX_VM_PROFILING|CMAKE_GENERATOR):' "$release_dir/CMakeCache.txt"
    shasum -a 256 "$release_dir/bin/rxc" "$release_dir/bin/rxas" \
      "$release_dir/bin/rxlink" "$release_dir/bin/rxvm" "$release_dir/bin/rxbvm" \
      "$release_dir/bin/library.rxbin" "$image_base.rxas" "$image_base.rxbin"
    print 'dirty_worktree:'
    git -C "$repo_root" status --short
  } > "$script_dir/provenance.txt"
}

capture_profiles() {
  local profile_image="$profiles_dir/candidate-rexxcps-opt"
  local vm_name
  local baseline_profile
  local candidate_profile
  rg -q '^CMAKE_BUILD_TYPE:STRING=Release$' "$profile_dir/CMakeCache.txt"
  rg -q '^CREXX_VM_PROFILING:BOOL=ON$' "$profile_dir/CMakeCache.txt"
  mkdir -p "$profiles_dir" "$profile_raw_dir"
  "$profile_dir/bin/rxc" -i "$profile_dir/bin" -o "$profile_image.rxas" "$source_path"
  "$profile_dir/bin/rxas" -o "$profile_image.rxbin" "$profile_image.rxas"
  for vm_name in rxvm rxbvm; do
    candidate_profile="$profiles_dir/canonical-opt-$vm_name.csv"
    "$profile_dir/bin/$vm_name" --profile-output "$candidate_profile" \
      "$profile_image.rxbin" "$profile_dir/bin/library.rxbin" \
      -a --smoke-count 1 \
      > "$profile_raw_dir/canonical-opt-$vm_name.stdout.txt" \
      2> "$profile_raw_dir/canonical-opt-$vm_name.stderr.txt"
    rg -q '^PASS: RexxCPS 2\.2d cREXX port$' \
      "$profile_raw_dir/canonical-opt-$vm_name.stdout.txt"
    rg -q 'contract=noncanonical-smoke' \
      "$profile_raw_dir/canonical-opt-$vm_name.stdout.txt"
    rg -q 'effective_count=1' \
      "$profile_raw_dir/canonical-opt-$vm_name.stdout.txt"
    baseline_profile="$accepted_dir/profiles/canonical-opt-$vm_name.csv"
    awk -F, '$1=="instruction" {print $2 "," $5}' "$baseline_profile" | sort \
      > "$profiles_dir/baseline-$vm_name-counts.csv"
    awk -F, '$1=="instruction" {print $2 "," $5}' "$candidate_profile" | sort \
      > "$profiles_dir/candidate-$vm_name-counts.csv"
    awk -F, -v vm="$vm_name" '
      FNR==NR {base[$1]=$2; names[$1]=1; next}
      {candidate[$1]=$2; names[$1]=1}
      END {for (name in names) printf "%s,%s,%d,%d,%+d\n", vm,name,base[name]+0,candidate[name]+0,(candidate[name]+0)-(base[name]+0)}' \
      "$profiles_dir/baseline-$vm_name-counts.csv" \
      "$profiles_dir/candidate-$vm_name-counts.csv" | sort \
      > "$profiles_dir/dynamic-$vm_name-deltas.csv"
  done
  print 'vm,opcode,baseline_count,candidate_count,delta' > "$script_dir/dynamic-opcode-deltas.csv"
  cat "$profiles_dir"/dynamic-*-deltas.csv >> "$script_dir/dynamic-opcode-deltas.csv"
  print 'vm,opcode,baseline_count,candidate_count,delta' \
    > "$script_dir/dynamic-new-instructions.csv"
  for vm_name in rxvm rxbvm; do
    awk -F, -v vm="$vm_name" '
      $1=="instruction" && (($4>=263 && $4<=299) || ($4>=401 && $4<=423)) {
        printf "%s,%s,0,%s,%+d\n", vm,$2,$5,$5
      }' "$profiles_dir/canonical-opt-$vm_name.csv" | sort \
      >> "$script_dir/dynamic-new-instructions.csv"
  done
  {
    shasum -a 256 "$profile_image.rxas" "$profile_image.rxbin" \
      "$profile_dir/bin/library.rxbin"
    shasum -a 256 "$script_dir/candidate-rexxcps-opt.rxas" \
      "$script_dir/candidate-rexxcps-opt.rxbin" "$release_dir/bin/library.rxbin"
  } > "$script_dir/profile-product-hashes.txt"
}

run_compat_sample() {
  local vm_name=$1
  local phase_name=$2
  local phase_sequence=$3
  local sequence=$4
  local compat_dir="$script_dir/compatibility"
  local stem
  local stdout_path
  local stderr_path
  local native_cps
  local process_real_s
  local effective_count
  stem=$(printf '%02d-%s-%s-%d' "$sequence" "$vm_name" "$phase_name" "$phase_sequence")
  stdout_path="$compat_dir/raw/$stem.stdout.txt"
  stderr_path="$compat_dir/raw/$stem.stderr.txt"
  /usr/bin/time -p "$release_dir/bin/$vm_name" \
    "$old_product_dir/canonical-opt.rxbin" \
    "$old_product_dir/library.rxbin" > "$stdout_path" 2> "$stderr_path"
  native_cps=$(awk '/Performance:/ {print $2}' "$stdout_path")
  process_real_s=$(awk '$1 == "real" {print $2}' "$stderr_path")
  effective_count=$(sed -n 's/.* effective_count=\([0-9][0-9]*\).*/\1/p' "$stdout_path")
  if [[ -z "$native_cps" || -z "$process_real_s" || \
        "$effective_count" != 100 ]] || \
     ! rg -q '^PASS: RexxCPS 2\.2d cREXX port$' "$stdout_path"; then
    print -u2 "invalid old-RXBIN compatibility sample: $stem"
    exit 1
  fi
  print "$sequence,$vm_name,$phase_name,$phase_sequence,$native_cps,$process_real_s,$effective_count,1,compatibility/raw/$stem.stdout.txt,compatibility/raw/$stem.stderr.txt" \
    >> "$compat_dir/samples.csv"
  print "$stem old-rxbin cps=$native_cps real=$process_real_s"
}

capture_old_rxbin_compatibility() {
  local compat_dir="$script_dir/compatibility"
  local expected_image_hash=466de7f06414148e4a3f63337e716ce332e1c082363297974240f4b3cc9cabbe
  local expected_library_hash=6d1ae40af463fd53633f44603a57cc2291aaa3149bb4ef0d04a67511ea04cf13
  local vm_name
  local image_mode
  local sequence=0
  local recorded
  local stdout_path
  local stderr_path
  local cps_values
  local wall_values
  local baseline_row
  local candidate_row
  assert_release_tree
  [[ $(shasum -a 256 "$old_product_dir/canonical-opt.rxbin" | awk '{print $1}') == "$expected_image_hash" ]]
  [[ $(shasum -a 256 "$old_product_dir/library.rxbin" | awk '{print $1}') == "$expected_library_hash" ]]
  mkdir -p "$compat_dir/raw"

  print 'vm,image,pass,effective_count,stdout,stderr' > "$compat_dir/functional.csv"
  for vm_name in rxvm rxbvm; do
    for image_mode in noopt opt; do
      stdout_path="$compat_dir/raw/$vm_name-$image_mode-smoke.stdout.txt"
      stderr_path="$compat_dir/raw/$vm_name-$image_mode-smoke.stderr.txt"
      "$release_dir/bin/$vm_name" "$old_product_dir/canonical-$image_mode.rxbin" \
        "$old_product_dir/library.rxbin" -a --smoke-count 1 \
        > "$stdout_path" 2> "$stderr_path"
      rg -q '^PASS: RexxCPS 2\.2d cREXX port$' "$stdout_path"
      rg -q 'effective_count=1' "$stdout_path"
      print "$vm_name,canonical-$image_mode,1,1,compatibility/raw/${stdout_path:t},compatibility/raw/${stderr_path:t}" \
        >> "$compat_dir/functional.csv"
    done
  done

  print 'sequence,vm,phase,phase_sequence,native_cps,process_real_s,effective_count,pass,stdout,stderr' \
    > "$compat_dir/samples.csv"
  for vm_name in rxvm rxbvm; do
    (( sequence += 1 ))
    run_compat_sample "$vm_name" warmup 1 "$sequence"
    for recorded in 1 2 3; do
      (( sequence += 1 ))
      run_compat_sample "$vm_name" recorded "$recorded" "$sequence"
    done
  done

  print 'vm,n,native_cps_median,native_cps_mean,native_cps_min,native_cps_max,process_real_median_s,process_real_mean_s,process_real_min_s,process_real_max_s' \
    > "$compat_dir/summary.csv"
  for vm_name in rxvm rxbvm; do
    cps_values=$(mktemp /tmp/crexx-nr09-compat-cps.XXXXXX)
    wall_values=$(mktemp /tmp/crexx-nr09-compat-wall.XXXXXX)
    awk -F, -v vm="$vm_name" '$2==vm && $3=="recorded" {print $5}' "$compat_dir/samples.csv" | sort -n > "$cps_values"
    awk -F, -v vm="$vm_name" '$2==vm && $3=="recorded" {print $6}' "$compat_dir/samples.csv" | sort -n > "$wall_values"
    print "$vm_name,3,$(sed -n '2p' "$cps_values"),$(awk '{s+=$1} END {printf "%.3f",s/NR}' "$cps_values"),$(sed -n '1p' "$cps_values"),$(sed -n '3p' "$cps_values"),$(sed -n '2p' "$wall_values"),$(awk '{s+=$1} END {printf "%.3f",s/NR}' "$wall_values"),$(sed -n '1p' "$wall_values"),$(sed -n '3p' "$wall_values")" \
      >> "$compat_dir/summary.csv"
    rm -f -- "$cps_values" "$wall_values"
  done

  print 'vm,old_vm_cps_median,new_vm_cps_median,cps_delta_percent,old_vm_real_median_s,new_vm_real_median_s,real_delta_percent' \
    > "$compat_dir/comparison.csv"
  for vm_name in rxvm rxbvm; do
    baseline_row=$(awk -F, -v vm="$vm_name" '$1==vm {print;exit}' "$old_verdict_dir/candidate-summary.csv")
    candidate_row=$(awk -F, -v vm="$vm_name" '$1==vm {print;exit}' "$compat_dir/summary.csv")
    print -r -- "$baseline_row|$candidate_row" | awk -F'[|,]' '{
      printf "%s,%s,%s,%+.3f,%s,%s,%+.3f\n",
        $1,$3,$13,(($13/$3)-1)*100,$7,$17,(($17/$7)-1)*100
    }' >> "$compat_dir/comparison.csv"
  done
  {
    print "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    print "old_product_verdict=$old_verdict_dir"
    shasum -a 256 "$old_product_dir/canonical-noopt.rxbin" \
      "$old_product_dir/canonical-opt.rxbin" "$old_product_dir/library.rxbin" \
      "$release_dir/bin/rxvm" "$release_dir/bin/rxbvm"
  } > "$compat_dir/provenance.txt"
}

case "$phase" in
  baseline) capture_baseline ;;
  candidate) capture_candidate ;;
  profile) capture_profiles ;;
  compatibility) capture_old_rxbin_compatibility ;;
  *) print -u2 'usage: run_verdict.zsh baseline|candidate|profile|compatibility'; exit 2 ;;
esac
