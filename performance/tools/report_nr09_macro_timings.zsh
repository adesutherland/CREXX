#!/bin/zsh

set -eu

if (( $# < 3 || $# > 4 )); then
  echo "usage: report_nr09_macro_timings.zsh BASELINE_PROFILE_DIR CANDIDATE_PROFILE_DIR OUTPUT_DIR [REVIEW_MANIFEST]" >&2
  exit 2
fi

baseline_dir=${1:A}
candidate_dir=${2:A}
output_dir=${3:A}
script_dir=${0:A:h}
manifest=${4:-${script_dir:h}/manifests/nr09-macro-review-v1.tsv}
manifest=${manifest:A}

if [[ ! -f "$manifest" ]]; then
  echo "missing NR-09 review manifest: $manifest" >&2
  exit 1
fi

for vm in rxvm rxbvm; do
  for input in \
    "$baseline_dir/canonical-opt-$vm.csv" \
    "$candidate_dir/canonical-opt-$vm.csv"; do
    if [[ ! -f "$input" ]]; then
      echo "missing schema-4 profile: $input" >&2
      exit 1
    fi
  done
done

mkdir -p "$output_dir"

details="$output_dir/form-timings.csv"
components="$output_dir/component-timings.csv"
ledger="$output_dir/review-ledger.tsv"
report="$output_dir/report.md"
provenance="$output_dir/provenance.txt"

print -r -- 'vm,opcode,class,family,components,component_count,terminal_transition,coherence_shape,side_effect_policy,review_bar,producer,handler_shape,candidate_count,saved_dispatches,baseline_macro_count,baseline_macro_average_ns,candidate_macro_total_ns,candidate_macro_average_ns,baseline_component_average_ns,estimated_baseline_handler_total_ns,handler_delta_ns,handler_delta_percent,baseline_terminal_transition_average_ns,candidate_terminal_transition_average_ns,estimated_baseline_full_average_ns,candidate_full_average_ns,estimated_baseline_full_total_ns,candidate_full_total_ns,full_delta_ns,full_delta_percent,candidate_profile_total_ns,full_delta_share_ppm,candidate_macro_time_share_percent,component_averages_below_timer_min,candidate_macro_below_timer_min,profile_signal,observation_band,calculation_status,missing_components,timer_read_min_ns,baseline_instruction_count,candidate_instruction_count,temporary_register_policy' > "$details"

print -r -- 'vm,macro_opcode,component_position,component_opcode,macro_count,baseline_global_count,baseline_global_total_ns,baseline_average_ns,estimated_total_at_macro_count_ns,timer_read_min_ns,average_below_timer_min' > "$components"

for vm in rxvm rxbvm; do
  baseline="$baseline_dir/canonical-opt-$vm.csv"
  candidate="$candidate_dir/canonical-opt-$vm.csv"

  awk -v manifest_file="$manifest" \
      -v baseline_file="$baseline" \
      -v candidate_file="$candidate" \
      -v expected_vm="$vm" \
      -v component_output="$components" '
  BEGIN { FS = "\t" }
  function csv(value, result) {
    result = value "";
    gsub(/"/, "\"\"", result);
    if (result ~ /[,"]/ || result ~ /^[[:space:]]/ || result ~ /[[:space:]]$/) {
      return "\"" result "\"";
    }
    return result;
  }
  function exact_average(total, count) {
    return count > 0 ? total / count : 0;
  }
  function transition_name(kind) {
    if (kind == "sequential") return "sequential_same_frame";
    return kind;
  }
  function temporary_register_policy(op, effect_policy) {
    if (effect_policy == "trace-temp-preserved") return "trace-temporary-register-passed";
    if (effect_policy == "compiler-temp-elided") return "compiler-temporary-elided";
    if (op == "ILOADCOPY_REG_REG_INT" ||
        op == "FDIVSUB_REG_REG_REG_FLOAT" ||
        op == "ITOSCONCAT_REG_STRING_REG") return "temporary-register-passed";
    return "no-compiler-temporary-output";
  }
  FNR == 1 && FILENAME != manifest_file {
    FS = ",";
    $0 = $0;
  }
  FILENAME == manifest_file {
    if (FNR == 1) next;
    if (NF != 14) {
      print "invalid manifest field count at line " FNR ": " NF > "/dev/stderr";
      failed = 1;
      next;
    }
    form_count++;
    opcode[form_count] = $1;
    class[form_count] = $2;
    family[form_count] = $3;
    components[form_count] = $4;
    terminal[form_count] = $5;
    coherence[form_count] = $6;
    effects[form_count] = $7;
    review_bar[form_count] = $8;
    producer[form_count] = $9;
    handler[form_count] = $10;
    next;
  }
  FILENAME == baseline_file {
    if (FNR == 1) next;
    if ($1 == "summary") {
      bs[$2] = $3;
    } else if ($1 == "instruction") {
      bi_count[$2] = $5 + 0;
      bi_total[$2] = $6 + 0;
      baseline_instruction_count += $5 + 0;
      baseline_profile_total += $6 + 0;
    } else if ($1 == "transition") {
      bt_count[$2] = $5 + 0;
      bt_total[$2] = $6 + 0;
      baseline_profile_total += $6 + 0;
    }
    next;
  }
  FILENAME == candidate_file {
    if (FNR == 1) next;
    if ($1 == "summary") {
      cs[$2] = $3;
    } else if ($1 == "instruction") {
      ci_count[$2] = $5 + 0;
      ci_total[$2] = $6 + 0;
      candidate_instruction_count += $5 + 0;
      candidate_profile_total += $6 + 0;
    } else if ($1 == "transition") {
      ct_count[$2] = $5 + 0;
      ct_total[$2] = $6 + 0;
      candidate_profile_total += $6 + 0;
    }
    next;
  }
  END {
    if (form_count != 60) {
      print "expected 60 NR-09 forms but manifest has " form_count > "/dev/stderr";
      failed = 1;
    }
    if (bs["schema_version"] != 4 || cs["schema_version"] != 4) {
      print "both profiles must use schema version 4" > "/dev/stderr";
      failed = 1;
    }
    if (bs["vm_mode"] != expected_vm || cs["vm_mode"] != expected_vm) {
      print "profile vm_mode does not match " expected_vm > "/dev/stderr";
      failed = 1;
    }
    if (bs["result"] != 0 || cs["result"] != 0 ||
        bs["invalid_events"] != 0 || cs["invalid_events"] != 0 ||
        bs["counter_overflow"] != 0 || cs["counter_overflow"] != 0) {
      print "profile result/events/overflow validation failed for " expected_vm > "/dev/stderr";
      failed = 1;
    }
    if (failed) exit 1;

    baseline_timer = bs["timer_read_min_ns"] + 0;
    candidate_timer = cs["timer_read_min_ns"] + 0;
    timer_min = baseline_timer > candidate_timer ? baseline_timer : candidate_timer;
    baseline_seq_avg = exact_average(bt_total["sequential_same_frame"],
                                     bt_count["sequential_same_frame"]);

    for (i = 1; i <= form_count; i++) {
      op = opcode[i];
      macro_count = ci_count[op] + 0;
      baseline_macro_count = bi_count[op] + 0;
      baseline_macro_avg = exact_average(bi_total[op], baseline_macro_count);
      macro_total = ci_total[op] + 0;
      macro_avg = exact_average(macro_total, macro_count);
      component_count = split(components[i], part, ";");
      component_avg = 0;
      below_count = 0;
      missing = "";

      for (j = 1; j <= component_count; j++) {
        component = part[j];
        global_count = bi_count[component] + 0;
        global_total = bi_total[component] + 0;
        avg = exact_average(global_total, global_count);
        if (global_count == 0) {
          if (missing != "") missing = missing ";";
          missing = missing component;
        } else {
          component_avg += avg;
          if (avg < timer_min) below_count++;
        }
        estimated_component_total = avg * macro_count;
        print expected_vm "," op "," j "," component "," \
              macro_count "," global_count "," global_total "," \
              sprintf("%.6f", avg) "," \
              sprintf("%.3f", estimated_component_total) "," \
              timer_min "," (global_count > 0 && avg < timer_min ? 1 : 0) >> component_output;
      }

      transition = transition_name(terminal[i]);
      baseline_terminal_avg = exact_average(bt_total[transition], bt_count[transition]);
      candidate_terminal_avg = exact_average(ct_total[transition], ct_count[transition]);
      transition_missing = (bt_count["sequential_same_frame"] + 0 == 0 ||
                            bt_count[transition] + 0 == 0 ||
                            ct_count[transition] + 0 == 0);
      status = "complete";
      if (missing != "") status = "missing-components";
      if (transition_missing) status = status == "complete" ? "missing-transition" : status "+missing-transition";
      if (macro_count > 0 && !(op in ci_count)) status = "missing-candidate-macro";
      if (status == "complete" && terminal[i] == "call_enter_frame")
        status = "unmatched-call-population";

      estimated_handler_total = component_avg * macro_count;
      handler_delta = macro_total - estimated_handler_total;
      handler_delta_percent = (component_avg > 0 ? (macro_avg - component_avg) * 100.0 / component_avg : 0);

      baseline_transition_avg = (component_count - 1) * baseline_seq_avg + baseline_terminal_avg;
      baseline_full_avg = component_avg + baseline_transition_avg;
      candidate_full_avg = macro_avg + candidate_terminal_avg;
      baseline_full_total = baseline_full_avg * macro_count;
      candidate_full_total = candidate_full_avg * macro_count;
      full_delta = candidate_full_total - baseline_full_total;
      full_delta_percent = (baseline_full_avg > 0 ? (candidate_full_avg - baseline_full_avg) * 100.0 / baseline_full_avg : 0);
      delta_share_ppm = (candidate_profile_total > 0 ? full_delta * 1000000.0 / candidate_profile_total : 0);
      macro_time_share = (candidate_profile_total > 0 ? macro_total * 100.0 / candidate_profile_total : 0);
      saved_dispatches = macro_count * (component_count - 1);

      if (macro_count == 0) {
        signal = "inactive";
        observation = "zero";
      } else if (status == "unmatched-call-population") {
        signal = "exact-cell-required";
        observation = macro_count < 100 ? "1-99" : (macro_count < 1000 ? "100-999" : "1000+");
      } else if (status != "complete") {
        signal = "incomplete-baseline";
        observation = macro_count < 100 ? "1-99" : (macro_count < 1000 ? "100-999" : "1000+");
      } else {
        if (full_delta > 0.5) signal = "possible-slowdown";
        else if (full_delta < -0.5) signal = "indicated-saving";
        else signal = "flat";
        observation = macro_count < 100 ? "1-99" : (macro_count < 1000 ? "100-999" : "1000+");
      }

      printf "%s,%s,%s,%s,%s,%d,%s,%s,%s,%s,%s,%s,%d,%d,%d,%.6f,%.0f,%.6f,%.6f,%.3f,%.3f,%.6f,%.6f,%.6f,%.6f,%.6f,%.3f,%.3f,%.3f,%.6f,%.0f,%.6f,%.6f,%d,%d,%s,%s,%s,%s,%d,%d,%d,%s\n",
             expected_vm, op, class[i], family[i], components[i], component_count,
             terminal[i], coherence[i], effects[i], review_bar[i], producer[i], handler[i],
             macro_count, saved_dispatches, baseline_macro_count, baseline_macro_avg,
             macro_total, macro_avg, component_avg, estimated_handler_total,
             handler_delta, handler_delta_percent, baseline_terminal_avg,
             candidate_terminal_avg, baseline_full_avg, candidate_full_avg,
             baseline_full_total, candidate_full_total, full_delta, full_delta_percent,
             candidate_profile_total, delta_share_ppm, macro_time_share,
             below_count, (macro_count > 0 && macro_avg < timer_min ? 1 : 0),
             signal, observation, status, csv(missing), timer_min,
             baseline_instruction_count, candidate_instruction_count,
             temporary_register_policy(op, effects[i]);
    }
  }
  ' "$manifest" "$baseline" "$candidate" >> "$details"
done

awk -v manifest_file="$manifest" -v details_file="$details" '
BEGIN { FS = "\t"; OFS = "\t" }
FILENAME == manifest_file {
  if (FNR == 1) {
    for (i = 1; i <= NF; i++) mh[$i] = i;
    next;
  }
  n++;
  for (i = 1; i <= NF; i++) manifest[n, i] = $i;
  order[$1] = n;
  next;
}
FILENAME == details_file {
  if (FNR == 1) {
    FS = ",";
    for (i = 1; i <= NF; i++) dh[$i] = i;
    next;
  }
  key = $1 SUBSEP $2;
  count[key] = $13;
  saved[key] = $14;
  old_handler[key] = $19;
  macro_handler[key] = $18;
  handler_delta[key] = $22;
  old_full[key] = $25;
  macro_full[key] = $26;
  full_delta[key] = $30;
  share_ppm[key] = $32;
  macro_share[key] = $33;
  signal[key] = $36;
  observation[key] = $37;
  status[key] = $38;
  temporary_policy[key] = $43;
  next;
}
END {
  print "opcode", "class", "family", "components", "component_count", "terminal_transition",
        "coherence_shape", "side_effect_policy", "review_bar", "producer", "handler_shape",
        "rxvm_count", "rxvm_saved_dispatches", "rxvm_component_handler_avg_ns",
        "rxvm_macro_handler_avg_ns", "rxvm_handler_delta_percent", "rxvm_component_full_avg_ns",
        "rxvm_macro_full_avg_ns", "rxvm_full_delta_percent", "rxvm_full_delta_share_ppm",
        "rxvm_macro_time_share_percent", "rxvm_profile_signal", "rxvm_observation_band",
        "rxbvm_count", "rxbvm_saved_dispatches", "rxbvm_component_handler_avg_ns",
        "rxbvm_macro_handler_avg_ns", "rxbvm_handler_delta_percent", "rxbvm_component_full_avg_ns",
        "rxbvm_macro_full_avg_ns", "rxbvm_full_delta_percent", "rxbvm_full_delta_share_ppm",
        "rxbvm_macro_time_share_percent", "rxbvm_profile_signal", "rxbvm_observation_band",
        "coherence_review", "implementation_review", "decision", "review_notes",
        "temporary_register_policy";
  for (row = 1; row <= n; row++) {
    op = manifest[row, 1];
    component_count = split(manifest[row, 4], component, ";");
    rx = "rxvm" SUBSEP op;
    rb = "rxbvm" SUBSEP op;
    print op, manifest[row, 2], manifest[row, 3], manifest[row, 4], component_count,
          manifest[row, 5], manifest[row, 6], manifest[row, 7], manifest[row, 8],
          manifest[row, 9], manifest[row, 10],
          count[rx] + 0, saved[rx] + 0, old_handler[rx], macro_handler[rx], handler_delta[rx],
          old_full[rx], macro_full[rx], full_delta[rx], share_ppm[rx], macro_share[rx],
          signal[rx], observation[rx],
          count[rb] + 0, saved[rb] + 0, old_handler[rb], macro_handler[rb], handler_delta[rb],
          old_full[rb], macro_full[rb], full_delta[rb], share_ppm[rb], macro_share[rb],
          signal[rb], observation[rb],
          manifest[row, 11], manifest[row, 12], manifest[row, 13], manifest[row, 14],
          temporary_policy[rx];
  }
}
' "$manifest" "$details" > "$ledger"

{
  print -r -- '# NR-09 macro timing and review report'
  print
  print -r -- 'This is a rerunnable decision aid for all 60 NR-09 forms. It compares each candidate macro handler with the sum of its legacy component-opcode averages and adds the corresponding transition costs.'
  print
  print -r -- '## Measurement boundary'
  print
  print -r -- '- These are schema-4 instrumented-profile signals, not ordinary Release verdicts. Instruction timing includes per-instruction profiler bookkeeping, so replacing several instructions with one macro mechanically removes several bookkeeping charges.'
  print -r -- '- The profile timer minimum is 41 ns while most short-handler averages are below that value. Counts and large totals are more useful than individual-call timings.'
  print -r -- '- Component averages are global opcode averages, not the exact old call sites replaced by each macro. Call and branch transition averages are global transition averages for the same reason.'
  print -r -- '- A call-bearing macro is marked `exact-cell-required`: its call cost is population-dependent, so a global `CALL` average cannot classify it as a saving or slowdown. Use a matched expanded/fused ordinary-Release cell.'
  print -r -- '- Per-form estimates overlap shared legacy opcode populations and must not be summed into a batch performance prediction.'
  print -r -- '- `possible-slowdown` and `indicated-saving` describe the instrumented estimate only. Ordinary profiling-off Release measurements remain decisive.'
  print
  print -r -- '## Coverage'
  print
  awk -F, '
    NR == 1 { next }
    {
      vm=$1;
      forms[vm]++;
      if ($13 > 0) used[vm]++;
      if ($36 == "possible-slowdown") slow[vm]++;
      if ($36 == "indicated-saving") saving[vm]++;
      if ($36 == "exact-cell-required") exact[vm]++;
      if ($37 == "1-99") low[vm]++;
      if (!seen_total[vm]++) {
        baseline_count[vm]=$41;
        candidate_count[vm]=$42;
        profile_total[vm]=$31;
      }
      dispatches[vm]+=$14;
    }
    END {
      print "| VM | Forms | Used | Zero-use | Indicated saving | Possible slowdown | Exact cell required | Used with <100 calls | Macro-model retired | Actual count drop | Residual | Profile instruction count |";
      print "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |";
      for (i=1;i<=2;i++) {
        vm=(i==1?"rxvm":"rxbvm");
        actual=baseline_count[vm]-candidate_count[vm];
        printf "| `%s` | %d | %d | %d | %d | %d | %d | %d | %d | %d | %+d | %d -> %d |\n",
               vm, forms[vm], used[vm], forms[vm]-used[vm], saving[vm], slow[vm], exact[vm], low[vm],
               dispatches[vm], actual, dispatches[vm]-actual,
               baseline_count[vm], candidate_count[vm];
      }
    }
  ' "$details"
  print
  print -r -- 'The exact per-component global totals/averages are in `component-timings.csv`; all calculated per-form values are in `form-timings.csv`. The reviewable source of truth is `review-ledger.tsv`.'
  print
  print -r -- '## Dynamically observed forms'
  print
  print -r -- '| Form | Class | Family | Components | Temporary-register policy | Review bar | Calls | Full delta rxvm / rxbvm | Delta share ppm rxvm / rxbvm | Signal |'
  print -r -- '| --- | ---: | --- | ---: | --- | --- | ---: | ---: | ---: | --- |'
  awk -F '\t' '
    NR == 1 { next }
    $12 > 0 || $24 > 0 {
      printf "| `%s` | %s | %s | %s | %s | %s | %s/%s | %s%% / %s%% | %s / %s | %s / %s |\n",
             $1,$2,$3,$5,$40,$9,$12,$24,$19,$31,$20,$32,$22,$34;
    }
  ' "$ledger"
  print
  print -r -- '## First timing-review candidates'
  print
  print -r -- 'These forms have at least 100 observations and a `possible-slowdown` signal in one or both VMs. They are investigation priorities, not removal verdicts.'
  print
  print -r -- '| Form | Calls | Full delta rxvm / rxbvm | Delta share ppm rxvm / rxbvm | Temporary-register policy |'
  print -r -- '| --- | ---: | ---: | ---: | --- |'
  awk -F '\t' '
    NR == 1 { next }
    ($12 >= 100 || $24 >= 100) && ($22 == "possible-slowdown" || $34 == "possible-slowdown") {
      printf "| `%s` | %s/%s | %s%% / %s%% | %s / %s | %s |\n",
             $1,$12,$24,$19,$31,$20,$32,$40;
    }
  ' "$ledger"
  print
  print -r -- '## Zero-use forms in this profile'
  print
  awk -F '\t' '
    NR == 1 { next }
    $12 == 0 && $24 == 0 {
      if (line != "") line=line ", ";
      line=line "`" $1 "`";
      count++;
      if (count % 6 == 0) { print line; print ""; line="" }
    }
    END { if (line != "") print line }
  ' "$ledger"
  print
  print -r -- 'Zero use means this canonical profile cannot decide the form. A targeted ordinary-Release family/isolation test is required before keeping or removing it on performance grounds.'
  print
  print -r -- '## Forms with a raised review bar'
  print
  print -r -- '| Form | Reason | Calls rxvm/rxbvm | Profile signal | Review status |'
  print -r -- '| --- | --- | ---: | --- | --- |'
  awk -F '\t' '
    NR == 1 { next }
    $9 != "normal" {
      printf "| `%s` | %s; %s; %s | %s/%s | %s / %s | coherence=%s; implementation=%s; decision=%s |\n",
             $1,$7,$9,$40,$12,$24,$22,$34,$36,$37,$38;
    }
  ' "$ledger"
  print
  print -r -- '## Per-form decision rule'
  print
  print -r -- 'For each form, review in this order: (1) ordinary-Release saving and dynamic relevance, (2) whether the opcode is a coherent semantic unit, (3) whether a temporary register is passed merely to retain a component effect, and (4) whether the handler is the fastest correct implementation. Record the result in the manifest review columns and rerun this report.'
} > "$report"

{
  print -r -- "manifest=$manifest"
  print -r -- "baseline_dir=$baseline_dir"
  print -r -- "candidate_dir=$candidate_dir"
  print -r -- "output_dir=$output_dir"
  print -r -- "command=${0:A} ${(q)baseline_dir} ${(q)candidate_dir} ${(q)output_dir} ${(q)manifest}"
  print -r -- 'sha256:'
  shasum -a 256 "$manifest" \
    "$baseline_dir/canonical-opt-rxvm.csv" \
    "$baseline_dir/canonical-opt-rxbvm.csv" \
    "$candidate_dir/canonical-opt-rxvm.csv" \
    "$candidate_dir/canonical-opt-rxbvm.csv"
} > "$provenance"

print -r -- "$report"
