set -e
task_bin="$PWD/cmake-build-levell-product/bin"
task_work="$PWD/cmake-build-levell-product/examples/calculator"
mkdir -p "$task_work/imports"
cmake -DLEVEL_L="$task_bin/levell" \
  -DSPEC="$PWD/lib/rxfnsl/examples/calculator/calculator.levell" \
  -DOUTPUT="$task_work/levell_calculator.rxas" -DFORMAT=rxas \
  -P lib/rxfnsl/tools/generate.cmake
"$task_bin/rxas" "$task_work/levell_calculator"
cp "$task_bin/library.rxbin" "$task_bin/rxfnsl.rxbin" \
  "$task_work/levell_calculator.rxbin" "$task_work/imports/"
"$task_bin/rxc" -x --no-exe-import -i "$task_work/imports" \
  -o "$task_work/calculator" lib/rxfnsl/examples/calculator/calculator.crexx
"$task_bin/rxas" "$task_work/calculator"
"$task_bin/rxlink" -s -o "$task_work/calculator-linked" \
  "$task_work/calculator.rxbin" "$task_work/levell_calculator.rxbin" \
  "$task_bin/rxfnsl.rxbin" "$task_bin/library.rxbin"
"$task_bin/rxtvm" "$task_work/calculator-linked.rxbin" \
  -a lib/rxfnsl/examples/calculator/example.calc
