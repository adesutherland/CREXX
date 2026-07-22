#!/bin/zsh
set -eu

repo_root="${0:A:h:h:h:h}"
bundle_root="${0:A:h}"
nr08_root="$repo_root/performance/evidence/2026-07-17-nr-08-lifetime-poc"
historical_root="$repo_root/performance/evidence/2026-07-16-nr-05-call-census"
profile_build="$repo_root/cmake-build-profile"
profile_vm="$profile_build/bin/rxvm"
rxseq="$profile_build/bin/rxseq"

mkdir -p "$bundle_root/current/entries" "$bundle_root/current/inputs"

for revision in baseline candidate; do
  mkdir -p "$bundle_root/current/inputs/$revision"
  cp "$nr08_root/$revision/artifacts/canonical-noopt.rxbin" \
    "$bundle_root/current/inputs/$revision/rexxcps-noopt.rxbin"
  cp "$nr08_root/$revision/artifacts/canonical-opt.rxbin" \
    "$bundle_root/current/inputs/$revision/rexxcps-opt.rxbin"
  cp "$nr08_root/$revision/artifacts/library.rxbin" \
    "$bundle_root/current/inputs/$revision/library.rxbin"
done

patch_hash=$(shasum -a 256 "$nr08_root/candidate/compiler-poc.patch" | awk '{print $1}')
baseline_revision="rexxcps-2.2d-baseline-e748621d1"
candidate_revision="rexxcps-2.2d-nr08-${patch_hash[1,12]}"

metadata="$bundle_root/entry-metadata.txt"
{
  print -- "crexx-nr09-entry-metadata|1"
  print -- "# entry|revision|role|workload|variant|mode|source_trace|evidence_link|image_sha256|module_sha256s"
  while IFS='|' read -r entry pair workload variant mode image module_paths args expectation source_trace warmups runs image_hash module_hashes; do
    [[ -z "$entry" || "$entry" == \#* || "$entry" == crexx-* ]] && continue
    print -- "$entry|nr05-6a064499f327-rxseq-schema4|historical|$pair|$variant|$mode|$source_trace|performance/evidence/2026-07-16-nr-05-call-census/entries/$entry/rxseq|$image_hash|$module_hashes"
  done < "$repo_root/performance/manifests/nr05-call-census-v1.txt"
} > "$metadata"

combined="$bundle_root/current/sequence-ranking.csv"
print -- "entry,window,rank,count,sites,modules,symbols,status,pattern,mapping,example_module,example_start" > "$combined"
command_log="$bundle_root/current/commands.txt"
print -- "# Exact current RexxCPS 2.2d and frozen NR-08 RXSEQ commands" > "$command_log"

for revision in baseline candidate; do
  if [[ "$revision" == baseline ]]; then
    revision_id="$baseline_revision"
    role="current"
    variant="baseline"
  else
    revision_id="$candidate_revision"
    role="candidate"
    variant="nr08-poc"
  fi

  library="$bundle_root/current/inputs/$revision/library.rxbin"
  library_hash=$(shasum -a 256 "$library" | awk '{print $1}')
  for mode in noopt opt; do
    entry="rexxcps-2.2d-$revision-$mode"
    entry_dir="$bundle_root/current/entries/$entry"
    mkdir -p "$entry_dir"
    image="$bundle_root/current/inputs/$revision/rexxcps-$mode.rxbin"
    image_hash=$(shasum -a 256 "$image" | awk '{print $1}')
    print -- "$entry|$revision_id|$role|rexxcps|$variant|$mode|source=tests/benchmarks/rexxcps_levelb.crexx;TRACE=Off;contract=noncanonical-smoke;smoke_count=1|performance/evidence/2026-07-17-nr-09-sequence-ledger-poc/current/entries/$entry|$image_hash|$library_hash" >> "$metadata"

    for window in 2 3 4; do
      sequence_file="$entry_dir/n$window.rxseq"
      candidates="$entry_dir/n$window-candidates.csv"
      print -r -- "$profile_vm --sequence-count=$window --sequence-output $sequence_file $image $library -a --smoke-count 1" >> "$command_log"
      "$profile_vm" --sequence-count="$window" --sequence-output "$sequence_file" \
        "$image" "$library" -a --smoke-count 1 \
        > "$entry_dir/n$window.stdout.txt" \
        2> "$entry_dir/n$window.stderr.txt"
      rg -q "PASS: RexxCPS 2.2d cREXX port" "$entry_dir/n$window.stdout.txt"

      print -r -- "$rxseq $sequence_file $image $library --output $candidates" >> "$command_log"
      "$rxseq" "$sequence_file" "$image" "$library" --output "$candidates" \
        > "$entry_dir/n$window-analyse.stdout.txt" \
        2> "$entry_dir/n$window-analyse.stderr.txt"
      awk -v entry="$entry" -v window="$window" 'NR > 1 {
        print entry "," window "," $0
      }' "$candidates" >> "$combined"
    done
  done
done

{
  print -- "crexx-nr09-ledger-inputs|1"
  print -- "entries|performance/evidence/2026-07-17-nr-09-sequence-ledger-poc/entry-metadata.txt"
  print -- "effects|binutils/include/rxopeffects.h"
  print -- "sequence|performance/evidence/2026-07-16-nr-05-call-census/summary/sequence-ranking.csv"
  print -- "sequence|performance/evidence/2026-07-17-nr-09-sequence-ledger-poc/current/sequence-ranking.csv"
} > "$bundle_root/ledger-inputs.txt"

{
  print -- "captured_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print -- "branch=$(git -C "$repo_root" branch --show-current)"
  print -- "head=$(git -C "$repo_root" rev-parse HEAD)"
  print -- "baseline_revision=$baseline_revision"
  print -- "candidate_revision=$candidate_revision"
  print -- "nr08_patch_sha256=$patch_hash"
  print -- "historical_rows=$(($(wc -l < "$historical_root/summary/sequence-ranking.csv") - 1))"
  print -- "current_rows=$(($(wc -l < "$combined") - 1))"
  shasum -a 256 "$profile_vm" "$rxseq" "$metadata" "$combined"
} > "$bundle_root/current/capture-state.txt"

(
  cd "$bundle_root"
  find . -type f ! -name checksums.sha256 -print | LC_ALL=C sort | \
    sed 's#^\./##' | xargs shasum -a 256
) > "$bundle_root/checksums.sha256"

print -- "PASS: retained exact current/NR-08 RexxCPS RXSEQ evidence under $bundle_root/current"
