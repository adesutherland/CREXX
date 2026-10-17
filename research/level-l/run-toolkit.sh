#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/cmake-build-release/bin"}
task_mode=${1:-opt}
task_group=${2:-all}
case "$task_group" in all|toolkit|squirrel|lr|earley|munch|c14cost|tabpeg|pratt|repair|front|stage|regional|authoring|front-stage|migration|schedule|pikalower|witness|lrrepair|pikalex|boundary|format) ;; *) printf 'Unknown focused group\n' >&2; exit 2;; esac
case "$task_mode" in opt|noopt) ;; *) printf 'Expected opt or noopt\n' >&2; exit 2;; esac
task_build="$LEVEL_L_RUN_ROOT/build/toolkit-$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-levell-toolkit.XXXXXX)
printf 'Build log: %s\n' "$task_log"
task_flags=(--no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
source "$task_dir/bootstrap-front.sh"
compile() {
  if level_l_is_product "$1"; then return; fi
  "$task_bin/rxc" "${task_flags[@]}" -x -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then
    "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else
    "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1
  fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
task_inputs=()
level_l_reuse_products
task_modules=(llcore llcheck llcap llhistory llpeg llsquirrel llcfg lllr llforest llearley llmunch llc14cost lltabpeg llproject llpratt llrepair llstage llfront llregional llgroup llschedule llpika llpikalower llpikatree llwitness lllexplain lllrrepair llboundary)
if [[ "$task_group" == toolkit ]]; then task_modules=(llcore llcheck llcap llpeg); fi
for task_module in "${task_modules[@]}"; do
  task_source="$task_dir/src/$task_module.crexx"
  if [[ ! -f "$task_source" ]]; then task_source="$task_root/lib/rxfnsl/rexx/$task_module.crexx"; fi
  compile "$task_module" "$task_source"
  if ! level_l_is_product "$task_module"; then task_inputs+=("$task_build/$task_module.rxbin"); fi
done
if [[ "$task_group" == all || "$task_group" == munch || "$task_group" == witness || "$task_group" == pikalex || "$task_group" == boundary || "$task_group" == format ]]; then
  for task_module in lldfa lldfabuild; do
    task_source="$task_dir/src/$task_module.crexx"
  if [[ ! -f "$task_source" ]]; then task_source="$task_root/lib/rxfnsl/rexx/$task_module.crexx"; fi
  compile "$task_module" "$task_source"
    if ! level_l_is_product "$task_module"; then task_inputs+=("$task_build/$task_module.rxbin"); fi
  done
  compile lexgen "$task_dir/tools/lexgen.crexx"
  for task_spec in common rollback priority shadow keyword config-sections; do
    "$task_bin/rxtvm" "$task_build/lexgen.rxbin" "$task_build/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/$task_spec.lex" "gl${task_spec//-/}" dfa > "$task_build/gl${task_spec//-/}.raw" 2>> "$task_log"
    sed 's/expose lex$/expose lex tabledata/' "$task_build/gl${task_spec//-/}.raw" > "$task_build/gl${task_spec//-/}.crexx"
    cat >> "$task_build/gl${task_spec//-/}.crexx" <<'ACCESSOR'
tabledata: procedure = .binary expose DATA
  return DATA
ACCESSOR
    compile "gl${task_spec//-/}" "$task_build/gl${task_spec//-/}.crexx"
    task_inputs+=("$task_build/gl${task_spec//-/}.rxbin")
  done
fi
for task_case in toolkit squirrel lr earley munch c14cost tabpeg pratt repair front stage regional migration schedule pikalower witness lrrepair pikalex boundary format; do
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
    if [[ "$task_case" == squirrel && -f "$task_dir/cases/fixtures/squirrel-expected.txt" ]]; then
      "$task_bin/$task_vm" "$task_build/compare-squirrel-linked.rxbin" -a "$task_dir/cases/fixtures/squirrel-expected.txt" "$task_output" > "$task_build/compare-squirrel-$task_vm.out" 2>&1
      cat "$task_build/compare-squirrel-$task_vm.out"
    fi
  done
done
cp "$task_log" "$task_build/build.log"
