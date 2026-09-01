#!/bin/zsh

set -euo pipefail

script_dir=${0:a:h}
repo_root=${script_dir:h:h:h}
poc_dir="$repo_root/performance/evidence/2026-07-17-nr-08-lifetime-poc"
baseline_csv="$poc_dir/baseline/library-procedure-static-opcounts.csv"
candidate_csv="$poc_dir/candidate/library-procedure-static-opcounts.csv"
totals_csv="$script_dir/library-static-audit.csv"
deltas_csv="$script_dir/library-procedure-deltas.csv"
summary_txt="$script_dir/library-audit-summary.txt"

if [[ ! -f "$baseline_csv" || ! -f "$candidate_csv" ]]; then
  print -u2 'corrected retained library inventories are missing'
  exit 1
fi

baseline_rows=$(awk 'END {print NR - 1}' "$baseline_csv")
candidate_rows=$(awk 'END {print NR - 1}' "$candidate_csv")
if (( baseline_rows != candidate_rows )); then
  print -u2 "procedure inventory mismatch: $baseline_rows versus $candidate_rows"
  exit 1
fi

print 'metric,baseline,candidate,delta' > "$totals_csv"
for field in 3 4 5 6 7; do
  case "$field" in
    3) metric=null ;;
    4) metric=endlife ;;
    5) metric=copies ;;
    6) metric=unlink ;;
    7) metric=reference_ops ;;
  esac
  baseline_total=$(awk -F, -v field="$field" 'NR > 1 {sum += $field} END {print sum + 0}' "$baseline_csv")
  candidate_total=$(awk -F, -v field="$field" 'NR > 1 {sum += $field} END {print sum + 0}' "$candidate_csv")
  delta=$(( candidate_total - baseline_total ))
  print "$metric,$baseline_total,$candidate_total,$delta" >> "$totals_csv"
done

awk -F, '
  BEGIN { print "module,procedure,null_delta,endlife_delta,copies_delta,unlink_delta,reference_ops_delta" }
  FNR==NR {
    if (FNR > 1) {
      key=$1 SUBSEP $2
      baseline_null[key]=$3
      baseline_endlife[key]=$4
      baseline_copies[key]=$5
      baseline_unlink[key]=$6
      baseline_reference[key]=$7
    }
    next
  }
  FNR == 1 { next }
  {
    key=$1 SUBSEP $2
    null_delta=$3-baseline_null[key]
    endlife_delta=$4-baseline_endlife[key]
    copies_delta=$5-baseline_copies[key]
    unlink_delta=$6-baseline_unlink[key]
    reference_delta=$7-baseline_reference[key]
    if (null_delta || endlife_delta || copies_delta || unlink_delta || reference_delta)
      printf "%s,%s,%d,%d,%d,%d,%d\n", $1, $2, null_delta,
        endlife_delta, copies_delta, unlink_delta, reference_delta
  }
' "$baseline_csv" "$candidate_csv" > "$deltas_csv"

changed_procedures=$(awk 'END {print NR - 1}' "$deltas_csv")
endlife_delta=$(awk -F, 'NR > 1 {sum += $4} END {print sum + 0}' "$deltas_csv")
non_endlife_changes=$(awk -F, 'NR > 1 && ($3 || $5 || $6 || $7) {count++} END {print count + 0}' "$deltas_csv")
endlife_increases=$(awk -F, 'NR > 1 && $4 > 0 {count++} END {print count + 0}' "$deltas_csv")

{
  print "baseline_procedures=$baseline_rows"
  print "candidate_procedures=$candidate_rows"
  print "changed_procedures=$changed_procedures"
  print "endlife_delta=$endlife_delta"
  print "non_endlife_changes=$non_endlife_changes"
  print "endlife_increases=$endlife_increases"
  print 'analysis=label-aware; an instruction following a branch label is parsed from field 2'
} > "$summary_txt"

if (( non_endlife_changes != 0 || endlife_increases != 0 )); then
  print -u2 'NR-08 imported-library audit found an unexpected change'
  cat "$summary_txt" >&2
  exit 1
fi

if ! rg -q '^null,4293,4293,0$' "$totals_csv" || \
   ! rg -q '^endlife,8006,151,-7855$' "$totals_csv" || \
   ! rg -q '^copies,8478,8478,0$' "$totals_csv" || \
   ! rg -q '^unlink,1758,1758,0$' "$totals_csv" || \
   ! rg -q '^reference_ops,7,7,0$' "$totals_csv"; then
  print -u2 'NR-08 imported-library totals do not match the audited exact images'
  cat "$totals_csv" >&2
  exit 1
fi

print 'PASS: NR-08 imported-library static audit'
cat "$totals_csv"
cat "$summary_txt"
