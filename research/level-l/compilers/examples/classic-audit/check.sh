#!/bin/sh
# Check the auditor with small programs whose answers we can work out by hand.
set -eu
vm=$1
image=$2
runtime=$3
assets=$4
scratch=$(mktemp -d "${TMPDIR:-/tmp}/classic-audit-check.XXXXXX")
cleanup() {
  status=$?
  trap - 0
  cmake -E rm -rf "$scratch"
  exit "$status"
}
trap cleanup 0
trap 'exit 130' INT
trap 'exit 143' HUP TERM
failures=0

# Each check names the behaviour and the sentence we expect the audit to print.
check() {
  title=$1
  source=$2
  expected=$3
  if output=$("$vm" "$image" -a "$source" "$runtime" 2>&1); then
    case "$output" in
      *"$expected"*) printf 'PASS: %s\n' "$title" ;;
      *) printf 'FAIL: %s\nExpected: %s\nGot:\n%s\n' "$title" "$expected" "$output"; failures=$((failures + 1)) ;;
    esac
  else
    printf 'FAIL: %s\nThe auditor could not read the example:\n%s\n' "$title" "$output"
    failures=$((failures + 1))
  fi
}

check 'A zero check before division protects the divisor.' "$assets/average.rexx" \
  'division by COUNT is protected by the zero check on line 2'
check 'Changing the divisor makes the earlier check stale.' "$assets/average-changed.rexx" \
  'COUNT is changed on line 3 after its zero check'
check 'A check after the division is too late.' "$assets/average-late.rexx" \
  'no covering zero check found for division by COUNT'
check 'A check outside a loop can become stale on its second iteration.' "$assets/average-loop.rexx" \
  'COUNT is changed on line 5 after its zero check'
check 'Updating the counter before ITERATE covers every restart.' "$assets/countdown.rexx" \
  'every route back to the test executes REMAINING = REMAINING - 1'
check 'ITERATE before the update leaves a route with no progress.' "$assets/countdown-skip.rexx" \
  'a route back to the test skips the decrement of REMAINING'
check 'Rejected readings still pass the counter update.' "$assets/readings.rexx" \
  'every route back to the test executes REMAINING = REMAINING - 1'
check 'The final average checks the accepted count.' "$assets/readings.rexx" \
  'division by ACCEPTED is protected by the zero check on line 13'
check 'The broken filter can restart on the same reading.' "$assets/readings-broken.rexx" \
  'a route back to the test skips the decrement of REMAINING'
check 'The broken filter can divide with no accepted readings.' "$assets/readings-broken.rexx" \
  'no covering zero check found for division by ACCEPTED'

cat > "$scratch/renamed.rexx" <<'REXX'
parse arg amount, items
if 0 = items then return 'empty'
return amount / items
REXX
check 'Renaming variables and reversing the comparison keeps the proof.' "$scratch/renamed.rexx" \
  'division by ITEMS is protected by the zero check on line 2'

cat > "$scratch/wrong-variable.rexx" <<'REXX'
parse arg total, count
if total = 0 then return 'empty'
return total / count
REXX
check 'Checking the numerator does not protect the denominator.' "$scratch/wrong-variable.rexx" \
  'no covering zero check found for division by COUNT'

cat > "$scratch/one-route.rexx" <<'REXX'
parse arg total, count
if total > 0 then do
  if count = 0 then return 'empty'
end
return total / count
REXX
check 'A check on just one route does not cover all routes.' "$scratch/one-route.rexx" \
  'no covering zero check found for division by COUNT'

cat > "$scratch/rejoining-route.rexx" <<'REXX'
parse arg total, count
if count = 0 then say 'empty'
return total / count
REXX
check 'Printing a warning does not stop the zero route reaching division.' "$scratch/rejoining-route.rexx" \
  'no covering zero check found for division by COUNT'

cat > "$scratch/other-write.rexx" <<'REXX'
parse arg total, count
if count = 0 then return 'empty'
total = total + 1
return total / count
REXX
check 'Changing another variable leaves the divisor check valid.' "$scratch/other-write.rexx" \
  'division by COUNT is protected by the zero check on line 2'

cat > "$scratch/new-check.rexx" <<'REXX'
parse arg total, count
if count = 0 then return 'empty'
count = count - 1
if count = 0 then return 'empty'
return total / count
REXX
check 'Checking again after an update protects the new value.' "$scratch/new-check.rexx" \
  'division by COUNT is protected by the zero check on line 4'

cat > "$scratch/nonzero-arm.rexx" <<'REXX'
parse arg total, count
if count <> 0 then return total / count
return 'empty'
REXX
check 'An explicit nonzero arm can protect a division too.' "$scratch/nonzero-arm.rexx" \
  'division by COUNT is protected by the zero check on line 2'

cat > "$scratch/parse-again.rexx" <<'REXX'
parse arg total, count
if count = 0 then return 'empty'
parse arg count
return total / count
REXX
check 'Parsing another argument can replace the checked value.' "$scratch/parse-again.rexx" \
  'COUNT is changed on line 3 after its zero check'

cat > "$scratch/other-exit.rexx" <<'REXX'
parse arg total, count
if count = 0 then return 'empty'
if total < 0 then do
  count = 0
  return 'negative total'
end
return total / count
REXX
check 'A write on a route that returns elsewhere does not spoil the proof.' "$scratch/other-exit.rexx" \
  'division by COUNT is protected by the zero check on line 2'

cat > "$scratch/check-each-time.rexx" <<'REXX'
parse arg total, count
do while total > 0
  if count = 0 then return 'empty'
  average = total / count
  count = count - 1
  total = total - 1
end
return average
REXX
check 'A fresh check inside the loop protects each division.' "$scratch/check-each-time.rexx" \
  'division by COUNT is protected by the zero check on line 3'

cat > "$scratch/fractional-check.rexx" <<'REXX'
parse arg total, count
if count = 0.5 then return 'half'
return total / count
REXX
check 'A comparison with one half must not be treated as a zero check.' "$scratch/fractional-check.rexx" \
  'no covering zero check found for division by COUNT'

cat > "$scratch/counter-reset.rexx" <<'REXX'
parse arg remaining
do while remaining > 0
  remaining = remaining - 1
  remaining = 10
end
return remaining
REXX
check 'An extra counter reset prevents the single-update conclusion.' "$scratch/counter-reset.rexx" \
  'REMAINING has other or multiple updates; progress is not established'

cat > "$scratch/fractional-step.rexx" <<'REXX'
parse arg remaining
do while remaining > 0
  remaining = remaining - 1.5
end
return remaining
REXX
check 'Subtracting one and a half must not be treated as subtracting one.' "$scratch/fractional-step.rexx" \
  'REMAINING has other or multiple updates; progress is not established'

printf 'parse arg total, count\rif count = 0 then return 0\rreturn total / count\r' > "$scratch/cr-lines.rexx"
check 'Source locations also work with carriage-return line endings.' "$scratch/cr-lines.rexx" \
  'Line 3: division by COUNT is protected by the zero check on line 2'

# Unsupported behaviour must stop the audit, not silently acquire a proof.
refuse() {
  title=$1
  source=$2
  expected=$3
  if output=$("$vm" "$image" -a "$source" "$runtime" 2>&1); then
    printf 'FAIL: %s\nThe auditor unexpectedly accepted the input:\n%s\n' "$title" "$output"
    failures=$((failures + 1))
    return
  fi
  case "$output" in
    *'is protected by'*|*'every route back to the test executes'*)
      printf 'FAIL: %s\nA proof was printed for rejected input:\n%s\n' "$title" "$output"
      failures=$((failures + 1)) ;;
    *"$expected"*) printf 'PASS: %s\n' "$title" ;;
    *) printf 'FAIL: %s\nExpected: %s\nGot:\n%s\n' "$title" "$expected" "$output"; failures=$((failures + 1)) ;;
  esac
}
cat > "$scratch/dynamic.rexx" <<'REXX'
parse arg total, count
if count = 0 then return 'empty'
interpret 'count = 0'
return total / count
REXX
refuse 'Dynamic code is reported instead of being assumed harmless.' "$scratch/dynamic.rexx" 'Cannot audit:'

printf 'if then return 0\n' > "$scratch/invalid.rexx"
refuse 'Malformed Rexx produces a parser diagnostic.' "$scratch/invalid.rexx" 'SYNTAX'

if [ "$failures" -ne 0 ]; then
  printf '%s checks failed.\n' "$failures"
  exit 1
fi
printf 'All checks passed.\n'
