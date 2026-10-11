#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_dir/build/frozen-tools-20261010"}
task_mode=${1:-opt}
case "$task_mode" in opt|noopt) ;; *) exit 2;; esac
task_build="$task_dir/build/rexxfront-$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp /tmp/crexx-levell-rexxfront.XXXXXX)
printf 'Build log: %s\n' "$task_log"
task_flags=(-x --no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
source "$task_dir/bootstrap-front.sh"
compile() {
  if [[ $1 == llfront ]]; then level_l_reuse_front; return; fi
  "$task_bin/rxc" "${task_flags[@]}" -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1; fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
task_inputs=()
for task_module in llcore llcheck llcap llhistory llpeg llcfg lllr llmunch lltabpeg llproject llstage llfront lldfa lldfabuild llcommentmask; do
  compile "$task_module" "$task_dir/src/$task_module.crexx"
  task_inputs+=("$task_build/$task_module.rxbin")
done
compile lexgen "$task_dir/tools/lexgen.crexx"
"$task_bin/rxtvm" "$task_build/lexgen.rxbin" "$task_build/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/rexx-fragment.lex" gcrexx dfa > "$task_build/gcrexx.raw" 2>> "$task_log"
sed 's/expose lex$/expose lex tabledata/' "$task_build/gcrexx.raw" > "$task_build/gcrexx.crexx"
cat >> "$task_build/gcrexx.crexx" <<'ACCESSOR'
tabledata: procedure = .binary expose DATA
  return DATA
ACCESSOR
compile gcrexx "$task_build/gcrexx.crexx"
compile rexxfront "$task_dir/cases/rexxfront.crexx"
"$task_bin/rxlink" -o "$task_build/rexxfront-linked" "$task_build/rexxfront.rxbin" "$task_build/gcrexx.rxbin" "${task_inputs[@]}" "$task_bin/library.rxbin" >> "$task_log" 2>&1
cd "$task_root"
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/rexxfront-linked.rxbin" > "$task_build/$task_vm.out" 2>&1
  tail -4 "$task_build/$task_vm.out"
  rg -q '^rexxfront_failures=0$' "$task_build/$task_vm.out"
done
cp "$task_log" "$task_build/build.log"
