#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/../CREXX/cmake-build-release/bin"}
task_re2c=${CREXX_RE2C:-"$task_bin/../re2c/re2c"}
task_pin=9a86fb37c0d0d7b9cd395d676e8a0a3ef45e39b8
task_boot=experiments/unicode/level-l-bootstrap
task_adapter=experiments/unicode/level-l-emitter
task_mode=${1:-opt}
task_nativebuild="$task_dir/build/$task_mode"
task_build="$task_dir/build/surface-$task_mode"
task_flags=(--no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags=(--no-exe-import -n); fi
mkdir -p "$task_build/imports" "$task_build/adapter"
task_log=$(mktemp /tmp/crexx-levell-surface.XXXXXX)
printf 'Authored-surface build log: %s\n' "$task_log"
# Read pinned Git objects; never edit or reset the other checkout.
for task_file in levell_bootstrap.crexx levell_generate.crexx specs/tinyexpr.levell; do
  git -C "$task_root" show "$task_pin:$task_boot/$task_file" > "$task_build/$(basename "$task_file")"
done
git -C "$task_root" show "$task_pin:$task_adapter/crexx.syntax" > "$task_build/adapter/crexx.syntax"
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
ln -sf "$task_bin/rxfnsl.rxbin" "$task_build/imports/rxfnsl.rxbin"
ln -sf "$task_nativebuild/lldfa.rxbin" "$task_build/imports/lldfa.rxbin"
compile() {
  "$task_bin/rxc" -x "${task_flags[@]}" -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then
    "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else
    "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1
  fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
compile levell_bootstrap "$task_build/levell_bootstrap.crexx"
compile levell_generate "$task_build/levell_generate.crexx"
compile surfacegen "$task_dir/tools/surfacegen.crexx"
task_providers=("$task_build/levell_bootstrap.rxbin" "$task_bin/library.rxbin")
"$task_bin/rxtvm" "$task_build/surfacegen.rxbin" "${task_providers[@]}" -a "$task_build/tinyexpr.levell" rules > "$task_build/tinyexpr.lex" 2>> "$task_log"
"$task_bin/rxtvm" "$task_build/surfacegen.rxbin" "${task_providers[@]}" -a "$task_build/tinyexpr.levell" wrapper > "$task_build/gsurface.crexx" 2>> "$task_log"
"$task_bin/rxtvm" "$task_nativebuild/lexgen.rxbin" "$task_nativebuild/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_build/tinyexpr.lex" gsurfaceraw dfa > "$task_build/gsurfaceraw.crexx" 2>> "$task_log"
compile gsurfaceraw "$task_build/gsurfaceraw.crexx"
compile gsurface "$task_build/gsurface.crexx"
"$task_bin/rxtvm" "$task_build/levell_generate.rxbin" "${task_providers[@]}" -a "$task_build/tinyexpr.levell" "$task_build/tinyexpr.re" "$task_build/tinyexpr.tokens" "$task_build/tinyexpr.ast" >> "$task_log" 2>&1
(cd "$task_build/adapter" && "$task_re2c" --lang python --syntax crexx.syntax --input-encoding utf8 --no-debug-info --no-generation-date --no-version -o "$task_build/level_l_tinyexpr.crexx" "$task_build/tinyexpr.re") >> "$task_log" 2>&1
compile level_l_tinyexpr "$task_build/level_l_tinyexpr.crexx"
compile surface "$task_dir/cases/surface.crexx"
"$task_bin/rxlink" -o "$task_build/surface-linked" "$task_build/surface.rxbin" "$task_build/gsurface.rxbin" "$task_build/gsurfaceraw.rxbin" "$task_build/level_l_tinyexpr.rxbin" "$task_nativebuild/lldfa.rxbin" "$task_bin/rxfnsl.rxbin" "$task_bin/library.rxbin" >> "$task_log" 2>&1
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/surface-linked.rxbin" > "$task_dir/evidence/surface-$task_mode-$task_vm.out" 2>&1
  cat "$task_dir/evidence/surface-$task_mode-$task_vm.out"
done
sed 's/U+0030 to U+0039/U+0100 to U+0109/' "$task_build/tinyexpr.levell" > "$task_build/wide.levell"
if "$task_bin/rxtvm" "$task_build/surfacegen.rxbin" "${task_providers[@]}" -a "$task_build/wide.levell" rules > "$task_dir/evidence/surface-$task_mode-unsupported.out" 2>&1; then
  printf 'FAIL: unsupported Unicode pattern silently accepted\n' >&2; exit 1
else
  task_status=$?
  if [[ "$task_status" != 1 ]]; then exit 1; fi
  printf 'PASS: unsupported Unicode backend feature rejected, authored contract preserved\n' >> "$task_dir/evidence/surface-$task_mode-unsupported.out"
fi
cp "$task_build/tinyexpr.levell" "$task_dir/specs/tinyexpr.levell"
cp "$task_build/tinyexpr.tokens" "$task_dir/evidence/tinyexpr-surface.tokens"
cp "$task_build/tinyexpr.ast" "$task_dir/evidence/tinyexpr-surface.ast"
cp "$task_log" "$task_dir/evidence/surface-$task_mode-build.log"
