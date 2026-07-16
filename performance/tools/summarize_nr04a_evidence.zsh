#!/bin/zsh

set -eu

if (( $# != 3 )); then
  echo "usage: summarize_nr04a_evidence.zsh BASELINE_BUNDLE CANDIDATE_BUNDLE OUTPUT_CSV" >&2
  exit 2
fi

baseline=$1/summary/paired-deltas.csv
candidate=$2/summary/paired-deltas.csv
output=$3

for input in "$baseline" "$candidate"; do
  if [[ ! -f "$input" ]]; then
    echo "missing paired summary: $input" >&2
    exit 1
  fi
done

awk -F, '
BEGIN {
  OFS=",";
  print "pair,baseline_retained_entry,baseline_stripped_entry,candidate_retained_entry,candidate_stripped_entry,baseline_retained_median_ns,baseline_stripped_median_ns,candidate_retained_median_ns,candidate_stripped_median_ns,baseline_retained_vs_stripped_percent,candidate_retained_vs_stripped_percent,retained_candidate_vs_baseline_percent,stripped_candidate_vs_baseline_percent";
}
NR == FNR {
  if (FNR > 1) {
    baseline_retained[$1]=$6;
    baseline_stripped[$1]=$7;
    baseline_retained_entry[$1]=$2;
    baseline_stripped_entry[$1]=$3;
    baseline_gap[$1]=$9;
  }
  next;
}
FNR > 1 {
  pair=$1;
  if (!(pair in baseline_retained)) {
    print "candidate pair missing from baseline: " pair > "/dev/stderr";
    failed=1;
    next;
  }
  retained_change=(($6-baseline_retained[pair])*100.0)/baseline_retained[pair];
  stripped_change=(($7-baseline_stripped[pair])*100.0)/baseline_stripped[pair];
  printf "%s,%s,%s,%s,%s,%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f\n", pair,
    baseline_retained_entry[pair], baseline_stripped_entry[pair], $2, $3,
    baseline_retained[pair], baseline_stripped[pair], $6, $7,
    baseline_gap[pair], $9, retained_change, stripped_change;
}
END { exit failed }
' "$baseline" "$candidate" > "$output"
