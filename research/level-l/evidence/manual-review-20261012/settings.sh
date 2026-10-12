set -e
task_bin="$PWD/cmake-build-levell-product/bin"
task_work="$PWD/cmake-build-levell-product/examples/settings"
mkdir -p "$task_work/imports"

cmake -DLEVEL_L="$task_bin/levell" \
  -DSPEC="$PWD/lib/rxfnsl/examples/settings/01-basic.levell" \
  -DOUTPUT="$task_work/levell_settings.rxas" -DFORMAT=rxas \
  -P lib/rxfnsl/tools/generate.cmake

"$task_bin/rxas" "$task_work/levell_settings"
cp "$task_bin/library.rxbin" "$task_bin/rxfnsl.rxbin" \
  "$task_work/levell_settings.rxbin" "$task_work/imports/"
"$task_bin/rxc" -x --no-exe-import -i "$task_work/imports" \
  -o "$task_work/settings-reader" lib/rxfnsl/examples/settings-reader.crexx
"$task_bin/rxas" "$task_work/settings-reader"
"$task_bin/rxlink" -o "$task_work/settings-reader-linked" \
  "$task_work/settings-reader.rxbin" "$task_work/levell_settings.rxbin" \
  "$task_bin/rxfnsl.rxbin" "$task_bin/library.rxbin"
"$task_bin/rxtvm" "$task_work/settings-reader-linked.rxbin" \
  -a lib/rxfnsl/examples/settings.txt
