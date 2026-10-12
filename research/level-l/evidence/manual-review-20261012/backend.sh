set -e
task_bin="$PWD/cmake-build-levell-product/bin"
task_work="$PWD/cmake-build-levell-product/examples/backend"
mkdir -p "$task_work/imports"
cmake -DLEVEL_L="$task_bin/levell" \
  -DSPEC="$PWD/lib/rxfnsl/examples/backends/ordered-values.levell" \
  -DOUTPUT="$task_work/levell_lesson.rxas" -DFORMAT=rxas \
  -P lib/rxfnsl/tools/generate.cmake
"$task_bin/rxas" "$task_work/levell_lesson"
cp "$task_bin/library.rxbin" "$task_bin/rxfnsl.rxbin" \
  "$task_work/levell_lesson.rxbin" "$task_work/imports/"
"$task_bin/rxc" -x --no-exe-import -i "$task_work/imports" \
  -o "$task_work/inspect" lib/rxfnsl/examples/backends/inspect.crexx
"$task_bin/rxas" "$task_work/inspect"
"$task_bin/rxlink" -s -o "$task_work/inspect-linked" \
  "$task_work/inspect.rxbin" "$task_work/levell_lesson.rxbin" \
  "$task_bin/rxfnsl.rxbin" "$task_bin/library.rxbin"
printf '%s' 'true' > "$task_work/input.txt"
"$task_bin/rxtvm" "$task_work/inspect-linked.rxbin" -a "$task_work/input.txt"
