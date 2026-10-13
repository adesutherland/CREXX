#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$root"
bin=${CREXX_BIN_DIR:-"$root/cmake-build-levell-product/bin"}
log=$(mktemp /tmp/levell-general-matrix.XXXXXX)
echo "$log"
exec >"$log" 2>&1
for mode in noopt opt; do
  core="$root/research/level-l/build/product-$mode"
  out="$root/research/level-l/build/generated-$mode"
  mkdir -p "$out"
  "$bin/rxtvm" "$core/levell.rxbin" -a lib/rxfnsl/tools/example.levell --rxas >"$out/levell_example.rxas"
  if [[ $mode == noopt ]]; then "$bin/rxas" -n "$out/levell_example"; else "$bin/rxas" "$out/levell_example"; fi
  flags=(-x --no-exe-import)
  if [[ $mode == noopt ]]; then flags+=(-n); fi
  "$bin/rxc" "${flags[@]}" -i "$core/imports;$out" -o "$out/testGenerated" lib/rxfnsl/tests_functional/testGenerated.crexx
  if [[ $mode == noopt ]]; then "$bin/rxas" -n "$out/testGenerated"; else "$bin/rxas" "$out/testGenerated"; fi
  inputs=()
  for name in llcore llcheck llcap llcfg lllr llmunch llproject llstage llfront llregular llsource lllanguage lltree; do inputs+=("$core/$name.rxbin"); done
  "$bin/rxlink" -o "$out/testGenerated-linked" "$out/testGenerated.rxbin" "$out/levell_example.rxbin" "${inputs[@]}" "$bin/library.rxbin"
  for vm in rxtvm rxbvm; do echo "$mode $vm"; "$bin/$vm" "$out/testGenerated-linked.rxbin" -a lib/rxfnsl/tools/example.levell; done
done
