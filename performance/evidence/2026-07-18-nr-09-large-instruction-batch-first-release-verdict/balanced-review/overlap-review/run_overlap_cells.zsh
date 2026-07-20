#!/bin/zsh

set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h:h:h}
release_dir="$repo_root/cmake-build-release"
template_dir="$script_dir/templates"
generated_dir="$script_dir/generated"
output_dir="$script_dir/outputs"
samples="$script_dir/samples.tsv"

"$script_dir/generate_overlap_cells.py"
mkdir -p "$generated_dir" "$output_dir"
rm -f "$generated_dir"/*(N) "$output_dir"/*(N)

print 'vm\tcomparison\tloop_count\tpadding_cells\tprocess_run\tpair\torder\tcurrent_us\talternative_us\tdelta_us\tdelta_per_call_ns\talternative_delta_percent' > "$samples"

{
  print -- "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print -- "repo_head=$(git -C "$repo_root" rev-parse HEAD)"
  print -- "host=$(uname -srm)"
  print -- "release_rxas_sha256=$(shasum -a 256 "$release_dir/bin/rxas" | awk '{print $1}')"
  print -- "release_rxvm_sha256=$(shasum -a 256 "$release_dir/bin/rxvm" | awk '{print $1}')"
  print -- "release_rxbvm_sha256=$(shasum -a 256 "$release_dir/bin/rxbvm" | awk '{print $1}')"
  print -- "build_boundary=ordinary profiling-off Release product"
  print -- "measurement_boundary=16 eight-byte alternative-procedure offsets; 3 processes per offset/VM; 5 alternating pairs per process; first pair warm-up; 12 recorded"
  print -- "assembly_boundary=rxas -n preserves both explicitly selected paths"
} > "$script_dir/provenance.txt"

padding_order=(0 8 4 12 2 10 6 14 1 9 5 13 3 11 7 15)
comparison_index=0
while IFS=$'\t' read -r comparison loop_count portfolio_executions; do
  [[ "$comparison" == "comparison" ]] && continue
  stem=${comparison:l}
  template="$template_dir/$stem.rxas.in"
  (( comparison_index += 1 ))
  for padding in $padding_order; do
    pad_stem=$(printf '%s-pad-%02d' "$stem" "$padding")
    source="$generated_dir/$pad_stem.rxas"
    image="$generated_dir/$pad_stem"
    awk -v padding="$padding" '
      $0 == "@PADDING@" { for (i=0; i<padding; i++) print "    cnop"; next }
      { print }
    ' "$template" > "$source"
    "$release_dir/bin/rxas" -n -o "$image" "$source"
    for process_run in 1 2 3; do
      if (( (comparison_index + padding + process_run) % 2 == 0 )); then
        vm_order=(rxvm rxbvm)
      else
        vm_order=(rxbvm rxvm)
      fi
      for vm in $vm_order; do
        output="$output_dir/$pad_stem-$vm-run-$process_run.txt"
        "$release_dir/bin/$vm" "$image.rxbin" > "$output" 2>&1
        awk -v vm="$vm" -v comparison="$comparison" -v padding="$padding" -v process_run="$process_run" -v loop_count="$loop_count" '
          $0 == "current_us" || $0 == "alternative_us" {
            label=$0
            getline
            value=$1+0
            if (++measurement % 2 == 1) { pair++; first[pair]=label }
            if (label == "current_us") current[pair]=value
            else alternative[pair]=value
          }
          END {
            if (pair != 5) exit 1
            for (i=2; i<=pair; i++) {
              order=(first[i] == "current_us" ? "current-first" : "alternative-first")
              delta=alternative[i]-current[i]
              percent=(alternative[i]/current[i]-1.0)*100.0
              printf "%s\t%s\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%d\t%.6f\t%+.6f\n", vm,comparison,loop_count,padding,process_run,i-1,order,current[i],alternative[i],delta,delta*1000.0/loop_count,percent
            }
          }
        ' "$output" >> "$samples"
      done
    done
  done
done < "$script_dir/manifest.tsv"

"$script_dir/summarize_overlap_cells.py"
rm -f "$generated_dir"/*.rxas(N) "$generated_dir"/*.rxbin(N) "$output_dir"/*.txt(N)
print -- "$script_dir/summary.tsv"
