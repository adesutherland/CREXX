#!/usr/bin/env bash
# Focused four-stage Level B verification. The caller owns the detached output.
set -euo pipefail

if [ "$#" -ne 2 ]; then
  echo "usage: $0 PRODUCT_BIN_DIRECTORY DETACHED_OUTPUT_DIRECTORY" >&2
  exit 2
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "$script_dir/../../.." && pwd)"
product="$(cd "$1" && pwd)"
work="$2"
mkdir -p "$work"
work="$(cd "$work" && pwd)"

"$product/rxc" -i "$repo_dir/docs/texttools;$product" -o "$work/bookhighlight" \
  "$repo_dir/docs/texttools/bookhighlight.crexx" > "$work/compile.log" 2>&1
"$product/rxas" -o "$work/bookhighlight" "$work/bookhighlight" \
  > "$work/assemble.log" 2>&1
"$product/rxc" -i "$work;$product" -o "$work/highlight" \
  "$repo_dir/docs/texttools/tests/highlight.crexx" > "$work/test-compile.log" 2>&1
"$product/rxas" -o "$work/highlight" "$work/highlight" \
  > "$work/test-assemble.log" 2>&1
"$product/rxlink" -o "$work/highlight-linked" "$work/highlight.rxbin" \
  "$work/bookhighlight.rxbin" "$product/library.rxbin" > "$work/test-link.log" 2>&1
"$product/rxvm" "$work/highlight-linked.rxbin" > "$work/test-run.log" 2>&1
cat "$work/test-run.log"
