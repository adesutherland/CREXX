#!/bin/zsh

set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h:h}
prior_rebaseline=${script_dir:h}/rebaseline
candidate_bin=$repo_root/cmake-build-release/bin
accepted_bin=$prior_rebaseline/artifacts/accepted
accepted_image=$prior_rebaseline/images/cell-a
source_path=$repo_root/tests/benchmarks/rexxcps_levelb.crexx
candidate_artifacts=$script_dir/artifacts/candidate
images_dir=$script_dir/images
raw_dir=$script_dir/raw
samples_csv=$script_dir/samples.csv
provenance_txt=$script_dir/provenance.txt
candidate_image=$images_dir/cell-c
started_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)

expected_source_sha256=2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6
expected_accepted_rxbin_sha256=fa2856eff33d2514536e01149c76e6160f8ab9632f2896db22fc5923a69dd11b
expected_accepted_library_sha256=ec8c8ae42793def24a952800164059cb52820db5f7621364e884673666240817
expected_accepted_rxvm_sha256=61978081370f6bfda97bbfc1f1c98b6b8faa77dd9bd34f8add8cc84e5161bd0c
expected_accepted_rxbvm_sha256=25bd6665cf909e8c73813a577eb1777bf236933f2c495cb023506c5c4859f979
expected_candidate_rxc_sha256=f82f9f17c6f8a87f31e6f990226cc0b4c8890e670e9de0e8c23caccaea02c2b5
expected_candidate_rxas_sha256=603afc111db221ee5d8a3d4ef2a640d3ad2000c66c06619759ae4e0acadce638
expected_candidate_rxvm_sha256=7bc6dbcd4724de59a26356209cfc3d65595a264178d8083d0920f551fc3de870
expected_candidate_rxbvm_sha256=8adcfa2de3ef527a451ee5bf696ff4ff1623866d1b109c3809bbd1b64e6f6146
expected_candidate_library_sha256=8a658a63019aedb99244ad71206882ef8e488aa4f79fbab64a77ed84654b0049
expected_candidate_image_rxas_sha256=9715bbd495821cd80fc12b99beecc8fb05adada691c50b0e87d5a7026bf37b6b
expected_candidate_image_rxbin_sha256=7a36cb4dee471daab0b406dc3788058746ab9f55bf3e21b431eda27d36250f9e

fail() {
  print -u2 -- "$*"
  exit 1
}

sha256() {
  shasum -a 256 "$1" | awk '{print $1}'
}

[[ ! -e $samples_csv ]] || fail "refusing to overwrite $samples_csv"
rg -q '^CMAKE_BUILD_TYPE:STRING=Release$' $repo_root/cmake-build-release/CMakeCache.txt || fail 'candidate is not Release'
rg -q '^CREXX_VM_PROFILING:BOOL=OFF$' $repo_root/cmake-build-release/CMakeCache.txt || fail 'candidate profiling is enabled'
git -C $repo_root diff --check

for artifact in rxc rxas rxvm rxbvm library.rxbin; do
  [[ -f $candidate_bin/$artifact ]] || fail "missing candidate $artifact"
done
for artifact in rxvm rxbvm library.rxbin; do
  [[ -f $accepted_bin/$artifact ]] || fail "missing accepted $artifact"
done
[[ -f $accepted_image.rxbin ]] || fail 'missing accepted image'

[[ $(sha256 $source_path) == $expected_source_sha256 ]] || fail 'source hash mismatch'
[[ $(sha256 $accepted_image.rxbin) == $expected_accepted_rxbin_sha256 ]] || fail 'accepted image hash mismatch'
[[ $(sha256 $accepted_bin/library.rxbin) == $expected_accepted_library_sha256 ]] || fail 'accepted library hash mismatch'
[[ $(sha256 $accepted_bin/rxvm) == $expected_accepted_rxvm_sha256 ]] || fail 'accepted rxvm hash mismatch'
[[ $(sha256 $accepted_bin/rxbvm) == $expected_accepted_rxbvm_sha256 ]] || fail 'accepted rxbvm hash mismatch'
[[ $(sha256 $candidate_bin/rxc) == $expected_candidate_rxc_sha256 ]] || fail 'candidate rxc hash mismatch'
[[ $(sha256 $candidate_bin/rxas) == $expected_candidate_rxas_sha256 ]] || fail 'candidate rxas hash mismatch'
[[ $(sha256 $candidate_bin/rxvm) == $expected_candidate_rxvm_sha256 ]] || fail 'candidate rxvm hash mismatch'
[[ $(sha256 $candidate_bin/rxbvm) == $expected_candidate_rxbvm_sha256 ]] || fail 'candidate rxbvm hash mismatch'
[[ $(sha256 $candidate_bin/library.rxbin) == $expected_candidate_library_sha256 ]] || fail 'candidate library hash mismatch'

mkdir -p $candidate_artifacts $images_dir $raw_dir
cp $candidate_bin/rxvm $candidate_artifacts/rxvm
cp $candidate_bin/rxbvm $candidate_artifacts/rxbvm
cp $candidate_bin/library.rxbin $candidate_artifacts/library.rxbin

$candidate_bin/rxc -i $candidate_bin -o $candidate_image $source_path
$candidate_bin/rxas -o $candidate_image $candidate_image.rxas
[[ $(sha256 $candidate_image.rxas) == $expected_candidate_image_rxas_sha256 ]] || fail 'candidate RXAS hash mismatch'
[[ $(sha256 $candidate_image.rxbin) == $expected_candidate_image_rxbin_sha256 ]] || fail 'candidate RXBIN hash mismatch'

{
  print "started_utc=$started_utc"
  print "repo_root=$repo_root"
  print "candidate_branch=$(git -C $repo_root branch --show-current)"
  print "candidate_head=$(git -C $repo_root rev-parse HEAD)"
  print "source_sha256=$(sha256 $source_path)"
  print "accepted_rxbin_sha256=$(sha256 $accepted_image.rxbin)"
  print "accepted_library_sha256=$(sha256 $accepted_bin/library.rxbin)"
  print "accepted_rxvm_sha256=$(sha256 $accepted_bin/rxvm)"
  print "accepted_rxbvm_sha256=$(sha256 $accepted_bin/rxbvm)"
  print "candidate_rxc_sha256=$(sha256 $candidate_bin/rxc)"
  print "candidate_rxas_sha256=$(sha256 $candidate_bin/rxas)"
  print "candidate_rxas_image_sha256=$(sha256 $candidate_image.rxas)"
  print "candidate_rxbin_sha256=$(sha256 $candidate_image.rxbin)"
  print "candidate_library_sha256=$(sha256 $candidate_artifacts/library.rxbin)"
  print "candidate_rxvm_sha256=$(sha256 $candidate_artifacts/rxvm)"
  print "candidate_rxbvm_sha256=$(sha256 $candidate_artifacts/rxbvm)"
  print 'schedule=12 recorded rounds; all six A/B/C permutations twice; reverse-symmetric; VM order alternates by round'
  print 'cell_A=accepted VM + accepted RXBIN + accepted library'
  print 'cell_B=corrected candidate VM + accepted RXBIN + accepted library'
  print 'cell_C=corrected candidate VM + corrected candidate RXBIN + corrected candidate library'
  print "uname=$(uname -a)"
  print "cpu=$(sysctl -n machdep.cpu.brand_string 2>/dev/null || true)"
  print "logical_cpus=$(sysctl -n hw.logicalcpu 2>/dev/null || true)"
  sw_vers
  uptime
  pmset -g batt 2>/dev/null || true
  cmake --version | head -n 1
  ninja --version | awk '{print "ninja " $0}'
  clang --version | head -n 1
  print 'candidate_build_options:'
  rg '^(CMAKE_BUILD_TYPE|CMAKE_C_FLAGS_RELEASE|CREXX_VM_PROFILING|CMAKE_GENERATOR):' $repo_root/cmake-build-release/CMakeCache.txt
  print 'candidate_dirty_worktree:'
  git -C $repo_root status --short
} > $provenance_txt

print 'sequence,vm_mode,phase,round,position,cell,vm_binary,image,library,native_cps,process_real_s,effective_count,pass,started_utc,ended_utc,stdout,stderr' > $samples_csv
sequence=0

run_sample() {
  local vm_mode=$1
  local phase=$2
  local round=$3
  local position=$4
  local cell=$5
  local vm_path image_base library_base stem stdout_path stderr_path
  local native_cps process_real_s effective_count pass sample_started sample_ended

  case $cell in
    A)
      vm_path=$accepted_bin/$vm_mode
      image_base=$accepted_image
      library_base=$accepted_bin/library
      ;;
    B)
      vm_path=$candidate_artifacts/$vm_mode
      image_base=$accepted_image
      library_base=$accepted_bin/library
      ;;
    C)
      vm_path=$candidate_artifacts/$vm_mode
      image_base=$candidate_image
      library_base=$candidate_artifacts/library
      ;;
    *) fail "unknown cell: $cell" ;;
  esac

  (( sequence += 1 ))
  stem=$(printf '%03d-%s-%s-r%02d-p%d-cell%s' $sequence $vm_mode $phase $round $position $cell)
  stdout_path=$raw_dir/$stem.stdout.txt
  stderr_path=$raw_dir/$stem.stderr.txt
  sample_started=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  /usr/bin/time -p $vm_path $image_base $library_base > $stdout_path 2> $stderr_path
  sample_ended=$(date -u +%Y-%m-%dT%H:%M:%SZ)

  native_cps=$(awk '/Performance:/ {print $2}' $stdout_path)
  process_real_s=$(awk '$1 == "real" {print $2}' $stderr_path)
  effective_count=$(sed -n 's/.* effective_count=\([0-9][0-9]*\).*/\1/p' $stdout_path)
  pass=0
  if rg -q '^PASS: RexxCPS 2\.2d cREXX port$' $stdout_path && \
     rg -q 'REXXCPS-PROVENANCE contract=canonical-default argv_count=0' $stdout_path; then
    pass=1
  fi
  [[ -n $native_cps && -n $process_real_s && $effective_count == 100 && $pass == 1 ]] || fail "invalid sample: $stem"

  print "$sequence,$vm_mode,$phase,$round,$position,$cell,$vm_path,$image_base.rxbin,$library_base.rxbin,$native_cps,$process_real_s,$effective_count,$pass,$sample_started,$sample_ended,raw/$stem.stdout.txt,raw/$stem.stderr.txt" >> $samples_csv
  print "$stem cps=$native_cps real=$process_real_s"
}

for vm_mode in rxvm rxbvm; do
  if [[ $vm_mode == rxvm ]]; then
    warmup_order=ABC
  else
    warmup_order=CBA
  fi
  for position in 1 2 3; do
    run_sample $vm_mode warmup 0 $position $warmup_order[$position]
  done
done

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
      run_sample $vm_mode recorded $round $position $order[$position]
    done
  done
done

print "ended_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)" >> $provenance_txt
python3 $script_dir/summarize_corrected.py $samples_csv $script_dir

{
  shasum -a 256 $source_path $accepted_image.rxbin \
    $accepted_bin/library.rxbin $accepted_bin/rxvm $accepted_bin/rxbvm \
    $candidate_image.rxas $candidate_image.rxbin \
    $candidate_artifacts/library.rxbin $candidate_artifacts/rxvm \
    $candidate_artifacts/rxbvm $samples_csv $script_dir/cell-summary.csv \
    $script_dir/paired-deltas.csv $script_dir/paired-summary.csv \
    $script_dir/position-summary.csv $provenance_txt
} > $script_dir/sha256sums.txt

print
print 'Cell summary:'
sed -n '1,7p' $script_dir/cell-summary.csv
print
print 'Paired summary:'
sed -n '1,7p' $script_dir/paired-summary.csv
