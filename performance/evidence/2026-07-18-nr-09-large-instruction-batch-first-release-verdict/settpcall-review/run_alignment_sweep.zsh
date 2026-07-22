#!/bin/zsh

set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h:h}
release_dir="$repo_root/cmake-build-release"
generated_dir="$script_dir/alignment-sweep"
template="$script_dir/alignment-sweep.rxas.in"
samples="$script_dir/alignment-sweep-samples.csv"
summary="$script_dir/alignment-sweep-summary.csv"
loop_count=5000000

[[ $(sysctl -n hw.cachelinesize) == 128 ]]
mkdir -p "$generated_dir"
print 'vm,padding_cells,pair,order,expanded_us,fused_us,delta_us,delta_per_call_ns' > "$samples"

padding_order=(0 8 4 12 2 10 6 14 1 9 5 13 3 11 7 15)

for padding in $padding_order; do
  stem=$(printf 'pad-%02d' "$padding")
  source="$generated_dir/$stem.rxas"
  image="$generated_dir/$stem"
  awk -v padding="$padding" -v loop_count="$loop_count" '
    $0 == "@PADDING@" {
      for (i = 0; i < padding; i++) print "    cnop";
      next;
    }
    {
      gsub(/@LOOP_COUNT@/, loop_count);
      print;
    }
  ' "$template" > "$source"
  "$release_dir/bin/rxas" -n -o "$image" "$source"

  if (( padding % 2 == 0 )); then
    vm_order=(rxvm rxbvm)
  else
    vm_order=(rxbvm rxvm)
  fi
  for vm in $vm_order; do
    output="$generated_dir/$stem-$vm.stdout.txt"
    "$release_dir/bin/$vm" "$image" > "$output"
    awk -v vm="$vm" -v padding="$padding" -v loop_count="$loop_count" '
      $0 == "expanded_us" || $0 == "fused_us" {
        label=$0;
        getline;
        value=$1;
        if (++measurement % 2 == 1) {
          pair++;
          first[pair]=label;
        }
        if (label == "expanded_us") expanded[pair]=value;
        else fused[pair]=value;
      }
      END {
        if (pair != 5) exit 1;
        for (i = 2; i <= pair; i++) {
          order=(first[i] == "expanded_us" ? "expanded-first" : "fused-first");
          delta=fused[i]-expanded[i];
          printf "%s,%d,%d,%s,%d,%d,%d,%.6f\n", vm, padding, i-1,
                 order, expanded[i], fused[i], delta,
                 delta * 1000.0 / loop_count;
        }
      }
    ' "$output" >> "$samples"
  done
done

print 'vm,padding_cells,n,mean_delta_us,mean_delta_per_call_ns' > "$summary"
awk -F, -v loop_count="$loop_count" '
  NR == 1 { next }
  {
    key=$1 SUBSEP $2;
    count[key]++;
    sum[key]+=$7;
    vm_count[$1]++;
    vm_sum[$1]+=$7;
  }
  END {
    for (vm_index=1; vm_index<=2; vm_index++) {
      vm=(vm_index == 1 ? "rxvm" : "rxbvm");
      for (padding=0; padding<16; padding++) {
        key=vm SUBSEP padding;
        printf "%s,%d,%d,%.6f,%.6f\n", vm, padding, count[key],
               sum[key]/count[key],
               (sum[key]/count[key]) * 1000.0 / loop_count;
      }
      printf "%s,all,%d,%.6f,%.6f\n", vm, vm_count[vm],
             vm_sum[vm]/vm_count[vm],
             (vm_sum[vm]/vm_count[vm]) * 1000.0 / loop_count;
    }
  }
' "$samples" >> "$summary"

print -- "$summary"
