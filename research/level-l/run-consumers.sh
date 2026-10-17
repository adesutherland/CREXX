#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/cmake-build-release/bin"}
task_mode=${1:-opt}
case "$task_mode" in opt|noopt) ;; *) exit 2;; esac
task_build="$LEVEL_L_RUN_ROOT/build/consumers-$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-levell-consumers.XXXXXX)
printf 'Build log: %s\n' "$task_log"
task_flags=(-x --no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
source "$task_dir/bootstrap-front.sh"
compile() {
  if level_l_is_product "$1"; then return; fi
  "$task_bin/rxc" "${task_flags[@]}" -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then
    "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else
    "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1
  fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
task_inputs=()
level_l_reuse_products
for task_module in llcore llcheck llcap llhistory llpeg llcfg lllr llmunch lltabpeg llproject llstage llfront lldfa lldfabuild; do
  task_source="$task_dir/src/$task_module.crexx"
  if [[ ! -f "$task_source" ]]; then task_source="$task_root/lib/rxfnsl/rexx/$task_module.crexx"; fi
  compile "$task_module" "$task_source"
  if ! level_l_is_product "$task_module"; then task_inputs+=("$task_build/$task_module.rxbin"); fi
done
compile lexgen "$task_dir/tools/lexgen.crexx"
for task_lang in pascal plm basic lisp prolog; do
  "$task_bin/rxtvm" "$task_build/lexgen.rxbin" "$task_build/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/consumer-$task_lang.lex" "gc$task_lang" dfa > "$task_build/gc$task_lang.raw" 2>> "$task_log"
  sed 's/expose lex$/expose lex tabledata/' "$task_build/gc$task_lang.raw" > "$task_build/gc$task_lang.crexx"
  cat >> "$task_build/gc$task_lang.crexx" <<'ACCESSOR'
tabledata: procedure = .binary expose DATA
  return DATA
ACCESSOR
  compile "gc$task_lang" "$task_build/gc$task_lang.crexx"
  task_inputs+=("$task_build/gc$task_lang.rxbin")
done
compile consumers "$task_dir/cases/consumers.crexx"
"$task_bin/rxlink" -o "$task_build/consumers-linked" "$task_build/consumers.rxbin" "${task_inputs[@]}" "$task_bin/library.rxbin" >> "$task_log" 2>&1
cd "$task_root"
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/consumers-linked.rxbin" > "$task_build/consumers-$task_vm.out" 2>&1
  tail -6 "$task_build/consumers-$task_vm.out"
  rg -q '^consumers_failures=0$' "$task_build/consumers-$task_vm.out"
done
cp "$task_log" "$task_build/build.log"
