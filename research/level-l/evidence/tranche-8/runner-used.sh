#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/../CREXX/cmake-build-release/bin"}
task_mode=${1:-opt}
task_group=${2:-all}
case "$task_group" in all|toolkit|squirrel|lr|earley|munch|c14cost|tabpeg|pratt|repair|front|stage|regional|authoring|front-stage|migration|schedule|pikalower|witness) ;; *) printf 'Unknown focused group\n' >&2; exit 2;; esac
case "$task_mode" in opt|noopt) ;; *) printf 'Expected opt or noopt\n' >&2; exit 2;; esac
task_build="$task_dir/build/toolkit-$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp /tmp/crexx-levell-toolkit.XXXXXX)
printf 'Build log: %s\n' "$task_log"
task_flags=(--no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
compile() {
  "$task_bin/rxc" "${task_flags[@]}" -x -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then
    "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else
    "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1
  fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
task_inputs=()
for task_module in llcore llcheck llcap llhistory llpeg llsquirrel llcfg lllr llforest llearley llmunch llc14cost lltabpeg llproject llpratt llrepair llstage llfront llregional llgroup llschedule llpika llpikalower llpikatree llwitness lllexplain; do
  compile "$task_module" "$task_dir/src/$task_module.crexx"
  task_inputs+=("$task_build/$task_module.rxbin")
done
if [[ "$task_group" == all || "$task_group" == munch || "$task_group" == witness ]]; then
  for task_module in lldfa lldfabuild; do
    compile "$task_module" "$task_dir/src/$task_module.crexx"
    task_inputs+=("$task_build/$task_module.rxbin")
  done
  compile lexgen "$task_dir/tools/lexgen.crexx"
  for task_spec in common rollback priority shadow keyword; do
    "$task_bin/rxtvm" "$task_build/lexgen.rxbin" "$task_build/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/$task_spec.lex" "gl$task_spec" dfa > "$task_build/gl$task_spec.raw" 2>> "$task_log"
    sed 's/expose lex$/expose lex tabledata/' "$task_build/gl$task_spec.raw" > "$task_build/gl$task_spec.crexx"
    cat >> "$task_build/gl$task_spec.crexx" <<'ACCESSOR'
tabledata: procedure = .binary expose DATA
  return DATA
ACCESSOR
    compile "gl$task_spec" "$task_build/gl$task_spec.crexx"
    task_inputs+=("$task_build/gl$task_spec.rxbin")
  done
fi
for task_case in toolkit squirrel lr earley munch c14cost tabpeg pratt repair front stage regional migration schedule pikalower witness; do
  if [[ "$task_group" == authoring ]]; then
    case "$task_case" in front|stage|regional) ;; *) continue;; esac
  elif [[ "$task_group" == front-stage ]]; then
    case "$task_case" in front|stage) ;; *) continue;; esac
  elif [[ "$task_group" != all && "$task_group" != "$task_case" ]]; then continue
  fi
  compile "$task_case" "$task_dir/cases/$task_case.crexx"
  "$task_bin/rxlink" -o "$task_build/$task_case-linked" "$task_build/$task_case.rxbin" "${task_inputs[@]}" "$task_bin/library.rxbin" >> "$task_log" 2>&1
  if [[ "$task_case" == squirrel ]]; then
    compile compare-squirrel "$task_dir/cases/compare-squirrel.crexx"
    "$task_bin/rxlink" -o "$task_build/compare-squirrel-linked" "$task_build/compare-squirrel.rxbin" "$task_bin/library.rxbin" >> "$task_log" 2>&1
  fi
  for task_vm in rxtvm rxbvm; do
    task_output="$task_build/$task_case-$task_vm.out"
    if [[ "$task_case" == toolkit ]]; then task_output="$task_build/$task_vm.out"; fi
    "$task_bin/$task_vm" "$task_build/$task_case-linked.rxbin" > "$task_output" 2>&1
    tail -4 "$task_output"
    rg -q "^${task_case}_failures=0$" "$task_output"
    if [[ "$task_case" == squirrel && -f "$task_dir/evidence/squirrel-expected.txt" ]]; then
      "$task_bin/$task_vm" "$task_build/compare-squirrel-linked.rxbin" -a "$task_dir/evidence/squirrel-expected.txt" "$task_output" > "$task_build/compare-squirrel-$task_vm.out" 2>&1
      cat "$task_build/compare-squirrel-$task_vm.out"
    fi
  done
done
cp "$task_log" "$task_build/build.log"
