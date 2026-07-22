#!/bin/zsh

set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h:h}
parent_dir=${script_dir:h}
profile_dir="$repo_root/cmake-build-profile"
image_dir="$parent_dir/static/candidate-rxbin"
manifest="$script_dir/portfolio-manifest.tsv"
form_manifest="$repo_root/performance/manifests/nr09-macro-review-v1.tsv"
rxops="$repo_root/binutils/include/rxops.h"
profiles_dir="$script_dir/profiles"
outputs_dir="$script_dir/outputs"
disassembly_dir="$script_dir/disassembly"
raw_static="$script_dir/static-raw.tsv"
raw_dynamic="$script_dir/dynamic-raw.tsv"
static_summary="$script_dir/static-occurrences.tsv"
dynamic_summary="$script_dir/dynamic-occurrences.tsv"
combined="$script_dir/portfolio-occurrences.tsv"
provenance="$script_dir/provenance.txt"

rxvm="$profile_dir/bin/rxvm"
rxdas="$profile_dir/bin/rxdas"
library="$profile_dir/bin/library.rxbin"

for required in "$manifest" "$form_manifest" "$rxops" "$rxvm" "$rxdas" "$library"; do
  [[ -f "$required" ]] || { print -u2 -- "missing required file: $required"; exit 1; }
done

mkdir -p "$profiles_dir" "$outputs_dir" "$disassembly_dir"
rm -f "$profiles_dir"/*.csv(N) "$outputs_dir"/*.txt(N) "$disassembly_dir"/*.rxas(N)

{
  print -- "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print -- "repo_head=$(git -C "$repo_root" rev-parse HEAD)"
  print -- "repo_status=$(git -C "$repo_root" status --short | wc -l | tr -d ' ') paths changed"
  print -- "host=$(uname -srm)"
  print -- "profile_vm=$rxvm"
  print -- "profile_vm_sha256=$(shasum -a 256 "$rxvm" | awk '{print $1}')"
  print -- "rxdas_sha256=$(shasum -a 256 "$rxdas" | awk '{print $1}')"
  print -- "library_sha256=$(shasum -a 256 "$library" | awk '{print $1}')"
  print -- "profile_boundary=CREXX_VM_PROFILING schema-4 counts; timing fields ignored"
  print -- "dynamic_vm_boundary=rxvm only; opcode execution counts are semantic and prior canonical rxvm/rxbvm counts matched"
  print -- "rexxcps_boundary=noncanonical bounded --smoke-count 1 occurrence census"
} > "$provenance"

print 'image\tworkload\tmode\topcode\tcount' > "$raw_static"
print 'image\tworkload\tmode\topcode\tcount' > "$raw_dynamic"

tail -n +2 "$manifest" | while IFS=$'\t' read -r id workload mode image args expect; do
  image_path="$image_dir/$image"
  profile="$profiles_dir/$id.csv"
  output="$outputs_dir/$id.txt"
  disassembly="$disassembly_dir/$id.rxas"
  [[ -f "$image_path" ]] || { print -u2 -- "missing portfolio image: $image_path"; exit 1; }

  "$rxdas" -o "$disassembly" "$image_path"
  awk -v image="$id" -v workload="$workload" -v mode="$mode" '
    function hex(c) {
      c=tolower(c)
      return index("0123456789abcdef", c)-1
    }
    function h2d(s, n, i) {
      n=0
      for (i=1; i<=length(s); i++) n=n*16+hex(substr(s,i,1))
      return n
    }
    FILENAME==ARGV[1] && /^X\(/ {
      line=$0
      sub(/^X\(/, "", line)
      split(line, field, /,[[:space:]]*/)
      by_id[field[2]+0]=field[1]
      next
    }
    FILENAME==ARGV[2] && /\* 0x[0-9a-f]+:[0-9a-f]+/ {
      line=$0
      sub(/^.*\* 0x[0-9a-f]+:/, "", line)
      sub(/[^0-9a-f].*$/, "", line)
      id=h2d(line)
      if (by_id[id] != "") count[by_id[id]]++
    }
    END {
      for (opcode in count)
        printf "%s\t%s\t%s\t%s\t%d\n", image, workload, mode, opcode, count[opcode]
    }
  ' "$rxops" "$disassembly" >> "$raw_static"

  # Deliberate zsh word splitting: the manifest's args field contains either a
  # single integer or the two-token RexxCPS smoke override.
  "$rxvm" --profile-output "$profile" "$image_path" "$library" -a ${=args} > "$output" 2>&1
  rg -Fq "$expect" "$output" || { print -u2 -- "missing PASS marker for $id"; exit 1; }
  awk -v image="$id" -v workload="$workload" -v mode="$mode" '
    FILENAME==ARGV[1] && FNR>1 {
      split($0, field, "\t")
      wanted[field[1]]=1
      next
    }
    FILENAME==ARGV[2] {
      split($0, field, ",")
      if (field[1]=="instruction" && wanted[field[2]])
        printf "%s\t%s\t%s\t%s\t%s\n", image, workload, mode, field[2], field[5]
    }
  ' "$form_manifest" "$profile" >> "$raw_dynamic"
done

# The library is shared by every portfolio cell. Count its static sites once,
# separately from program-image sites so it is not accidentally multiplied by
# 22 when assessing code-base breadth.
library_disassembly="$disassembly_dir/library.rxas"
"$rxdas" -o "$library_disassembly" "$library"
awk '
  function hex(c) {
    c=tolower(c)
    return index("0123456789abcdef", c)-1
  }
  function h2d(s, n, i) {
    n=0
    for (i=1; i<=length(s); i++) n=n*16+hex(substr(s,i,1))
    return n
  }
  FILENAME==ARGV[1] && /^X\(/ {
    line=$0
    sub(/^X\(/, "", line)
    split(line, field, /,[[:space:]]*/)
    by_id[field[2]+0]=field[1]
    next
  }
  FILENAME==ARGV[2] && /\* 0x[0-9a-f]+:[0-9a-f]+/ {
    line=$0
    sub(/^.*\* 0x[0-9a-f]+:/, "", line)
    sub(/[^0-9a-f].*$/, "", line)
    id=h2d(line)
    if (by_id[id] != "") count[by_id[id]]++
  }
  END { for (opcode in count) print opcode "\t" count[opcode] }
' "$rxops" "$library_disassembly" | sort > "$script_dir/library-static.tsv"

awk -F'\t' '
  BEGIN { OFS="\t" }
  FILENAME==ARGV[1] && FNR>1 { form[++n]=$1; class[$1]=$2; family[$1]=$3; next }
  FILENAME==ARGV[2] && FNR>1 {
    opcode=$4
    total[opcode]+=$5
    mode_total[opcode,$3]+=$5
    images[opcode]++
    if (!seen_workload[opcode,$2]++) workloads[opcode]++
  }
  FILENAME==ARGV[3] { library[$1]=$2; next }
  END {
    print "opcode","class","family","program_sites","noopt_sites","opt_sites","image_breadth","workload_breadth","library_sites"
    for (i=1; i<=n; i++) {
      opcode=form[i]
      print opcode,class[opcode],family[opcode],total[opcode]+0,
            mode_total[opcode,"noopt"]+0,mode_total[opcode,"opt"]+0,
            images[opcode]+0,workloads[opcode]+0,library[opcode]+0
    }
  }
' "$form_manifest" "$raw_static" "$script_dir/library-static.tsv" > "$static_summary"

awk -F'\t' '
  BEGIN { OFS="\t" }
  FILENAME==ARGV[1] && FNR>1 { form[++n]=$1; class[$1]=$2; family[$1]=$3; next }
  FILENAME==ARGV[2] && FNR>1 {
    opcode=$4
    total[opcode]+=$5
    mode_total[opcode,$3]+=$5
    images[opcode]++
    if (!seen_workload[opcode,$2]++) workloads[opcode]++
  }
  END {
    print "opcode","class","family","executions","noopt_executions","opt_executions","image_breadth","workload_breadth"
    for (i=1; i<=n; i++) {
      opcode=form[i]
      print opcode,class[opcode],family[opcode],total[opcode]+0,
            mode_total[opcode,"noopt"]+0,mode_total[opcode,"opt"]+0,
            images[opcode]+0,workloads[opcode]+0
    }
  }
' "$form_manifest" "$raw_dynamic" > "$dynamic_summary"

awk -F'\t' '
  BEGIN { OFS="\t" }
  FILENAME==ARGV[1] && FNR>1 {
    program[$1]=$4; noopt_sites[$1]=$5; opt_sites[$1]=$6
    static_images[$1]=$7; static_workloads[$1]=$8; library[$1]=$9
    order[++n]=$1; class[$1]=$2; family[$1]=$3
    next
  }
  FILENAME==ARGV[2] && FNR>1 {
    executions[$1]=$4; noopt_exec[$1]=$5; opt_exec[$1]=$6
    dynamic_images[$1]=$7; dynamic_workloads[$1]=$8
  }
  END {
    print "opcode","class","family","program_sites","noopt_sites","opt_sites","library_sites","static_image_breadth","static_workload_breadth","executions","noopt_executions","opt_executions","dynamic_image_breadth","dynamic_workload_breadth"
    for (i=1; i<=n; i++) {
      opcode=order[i]
      print opcode,class[opcode],family[opcode],program[opcode],noopt_sites[opcode],opt_sites[opcode],library[opcode],static_images[opcode],static_workloads[opcode],executions[opcode]+0,noopt_exec[opcode]+0,opt_exec[opcode]+0,dynamic_images[opcode]+0,dynamic_workloads[opcode]+0
    }
  }
' "$static_summary" "$dynamic_summary" > "$combined"

print -- "$combined"
