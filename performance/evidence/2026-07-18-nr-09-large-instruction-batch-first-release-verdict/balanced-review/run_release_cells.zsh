#!/bin/zsh

set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h:h}
release_dir="$repo_root/cmake-build-release"
cell_root="$script_dir/release-cells"
template_dir="$cell_root/templates"
generated_dir="$cell_root/generated"
output_dir="$cell_root/outputs"
samples="$script_dir/release-cell-samples.tsv"
provenance="$script_dir/release-cell-provenance.txt"

rxas="$release_dir/bin/rxas"
rxvm="$release_dir/bin/rxvm"
rxbvm="$release_dir/bin/rxbvm"

"$script_dir/generate_release_cells.py"
for required in "$rxas" "$rxvm" "$rxbvm"; do
  [[ -x "$required" ]] || { print -u2 -- "missing Release tool: $required"; exit 1; }
done

mkdir -p "$generated_dir" "$output_dir"
rm -f "$generated_dir"/*(N) "$output_dir"/*(N)
print 'vm\topcode\tloop_count\tpadding_cells\tprocess_run\tpair\torder\texpanded_us\tfused_us\tdelta_us\tdelta_per_call_ns\tfused_delta_percent' > "$samples"

{
  print -- "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print -- "repo_head=$(git -C "$repo_root" rev-parse HEAD)"
  print -- "host=$(uname -srm)"
  print -- "cacheline_bytes=$(sysctl -n hw.cachelinesize)"
  print -- "release_rxas_sha256=$(shasum -a 256 "$rxas" | awk '{print $1}')"
  print -- "release_rxvm_sha256=$(shasum -a 256 "$rxvm" | awk '{print $1}')"
  print -- "release_rxbvm_sha256=$(shasum -a 256 "$rxbvm" | awk '{print $1}')"
  print -- "build_boundary=ordinary profiling-off Release product"
  print -- "measurement_boundary=16 eight-byte fused-procedure offsets; 3 processes per offset/VM; 5 alternating pairs per process; first pair warm-up; 12 recorded"
  print -- "assembly_boundary=rxas -n preserves expanded component sequences"
  print -- "settpcall_boundary=reuses prior focused 20M-call diagnostic and decisive 5M-call x 16-offset dual-VM cell"
} > "$provenance"

padding_order=(0 8 4 12 2 10 6 14 1 9 5 13 3 11 7 15)
form_index=0
while IFS=$'\t' read -r opcode loop_count evidence; do
  [[ "$opcode" == "opcode" ]] && continue
  [[ "$evidence" == "new-16-offset-cell" ]] || continue
  stem=${opcode:l}
  template="$template_dir/$stem.rxas.in"
  [[ -f "$template" ]] || { print -u2 -- "missing template for $opcode"; exit 1; }
  (( form_index += 1 ))

  for padding in $padding_order; do
    pad_stem=$(printf '%s-pad-%02d' "$stem" "$padding")
    source="$generated_dir/$pad_stem.rxas"
    image="$generated_dir/$pad_stem"
    awk -v padding="$padding" '
      $0 == "@PADDING@" {
        for (i=0; i<padding; i++) print "    cnop"
        next
      }
      { print }
    ' "$template" > "$source"
    "$rxas" -n -o "$image" "$source"

    for process_run in 1 2 3; do
      if (( (form_index + padding + process_run) % 2 == 0 )); then
        vm_order=(rxvm rxbvm)
      else
        vm_order=(rxbvm rxvm)
      fi
      for vm in $vm_order; do
        output="$output_dir/$pad_stem-$vm-run-$process_run.txt"
        "$release_dir/bin/$vm" "$image.rxbin" > "$output" 2>&1
        awk -v vm="$vm" -v opcode="$opcode" -v padding="$padding" -v process_run="$process_run" -v loop_count="$loop_count" '
        $0 == "expanded_us" || $0 == "fused_us" {
          label=$0
          getline
          value=$1+0
          if (++measurement % 2 == 1) {
            pair++
            first[pair]=label
          }
          if (label == "expanded_us") expanded[pair]=value
          else fused[pair]=value
        }
        END {
          if (pair != 5) exit 1
          for (i=2; i<=pair; i++) {
            order=(first[i] == "expanded_us" ? "expanded-first" : "fused-first")
            delta=fused[i]-expanded[i]
            percent=(fused[i]/expanded[i]-1.0)*100.0
            printf "%s\t%s\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%d\t%.6f\t%+.6f\n", vm, opcode, loop_count, padding, process_run, i-1, order, expanded[i], fused[i], delta, delta*1000.0/loop_count, percent
          }
        }
        ' "$output" >> "$samples"
      done
    done
  done
done < "$script_dir/release-cell-manifest.tsv"

"$script_dir/summarize_release_cells.py"
rm -f "$generated_dir"/*.rxas(N) "$generated_dir"/*.rxbin(N) "$output_dir"/*.txt(N)
print -- "$script_dir/release-cell-summary.tsv"
