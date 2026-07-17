#!/bin/zsh
set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h}
cd "$repo_root"

tool_source=performance/tools/build_sequence_ledger.crexx
tool_rxas="$script_dir/tool-build/build_sequence_ledger_noopt"
tool_rxbin="$script_dir/tool-build/build_sequence_ledger_noopt.rxbin"
manifest="$script_dir/ledger-inputs.txt"
rxbvm_output="$script_dir/retained-rxbvm"
rxvm_output="$script_dir/retained-rxvm"

cmake-build-release/bin/rxc -i cmake-build-release/bin -n \
  -o "$tool_rxas" "$tool_source"
cmake-build-release/bin/rxas -n -o "$tool_rxbin" "$tool_rxas"

cmake-build-release/bin/rxbvm "$tool_rxbin" \
  cmake-build-release/bin/library.rxbin \
  cmake-build-release/lib/classlib/StringTreeMap.rxbin \
  -a --manifest "$manifest" --output-dir "$rxbvm_output"
cmake-build-release/bin/rxvm "$tool_rxbin" \
  cmake-build-release/bin/library.rxbin \
  cmake-build-release/lib/classlib/StringTreeMap.rxbin \
  -a --manifest "$manifest" --output-dir "$rxvm_output"

for output_name in sequence-ledger.csv decision-view.md run-state.txt checksums.sha256; do
  cmp "$rxbvm_output/$output_name" "$rxvm_output/$output_name"
done

test "$(wc -l < performance/evidence/2026-07-16-nr-05-call-census/summary/sequence-ranking.csv | tr -d ' ')" = 14010
test "$(wc -l < "$script_dir/current/sequence-ranking.csv" | tr -d ' ')" = 10312
test "$(wc -l < "$rxbvm_output/sequence-ledger.csv" | tr -d ' ')" = 11333
test -z "$(tail -n +2 "$rxbvm_output/sequence-ledger.csv" | cut -d, -f1 | sort | uniq -d)"

rg -Fq 'sieve-noopt,4,47,1,1,1,3,candidate,"STOI_REG(r1) | ICOPY_REG_REG(r2,r1) | UNLINK_REG(r1) | ILT_REG_REG_INT(r1,r2,c1)"' \
  performance/evidence/2026-07-16-nr-05-call-census/summary/sequence-ranking.csv
rg -Fq 'seq-4-4122963121@nr05-6a064499f327-rxseq-schema4,seq-4-4122963121,nr05-6a064499f327-rxseq-schema4,historical,"STOI_REG(r1) | ICOPY_REG_REG(r2,r1) | UNLINK_REG(r1) | ILT_REG_REG_INT(r1,r2,c1)",4,20,20,10,20,1,20,1,noopt;opt' \
  "$rxbvm_output/sequence-ledger.csv"
rg -Fq 'NR-08 current=2072983;candidate=0;delta=-2072983' \
  "$rxbvm_output/decision-view.md"

print 'PASS: NR-09 ledger outputs are deterministic and representative counts reconcile to retained RXSEQ rows'
