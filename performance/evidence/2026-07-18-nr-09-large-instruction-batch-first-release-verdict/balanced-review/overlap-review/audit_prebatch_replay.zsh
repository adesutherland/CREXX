#!/bin/zsh

set -euo pipefail

script_dir=${0:A:h}
repo_root=${script_dir:h:h:h:h:h}
review_dir=${script_dir:h}
verdict_dir=${review_dir:h}
baseline_dir="$verdict_dir/static/baseline"
manifest="$review_dir/portfolio-manifest.tsv"
form_manifest="$repo_root/performance/manifests/nr09-macro-review-v1.tsv"
rxops="$repo_root/binutils/include/rxops.h"
super_object="$repo_root/cmake-build-release/compiler/CMakeFiles/rxclib.dir/rxcp_emit_super.c.o"
rxas="$repo_root/cmake-build-release/bin/rxas"
rxdas="$repo_root/cmake-build-profile/bin/rxdas"
profile_vm="$repo_root/cmake-build-profile/bin/rxvm"
accepted_library="$verdict_dir/rebaseline/artifacts/accepted/library.rxbin"
tmp_dir=$(mktemp -d /tmp/nr09-overlap-replay.XXXXXX)
trap 'rm -rf "$tmp_dir"' EXIT

for required in "$manifest" "$form_manifest" "$rxops" "$super_object" \
                "$rxas" "$rxdas" "$profile_vm" "$accepted_library"; do
  [[ -f "$required" ]] || { print -u2 -- "missing required file: $required"; exit 1; }
done

mkdir "$tmp_dir/transformed" "$tmp_dir/rxbin" "$tmp_dir/disassembly" \
      "$tmp_dir/profiles" "$tmp_dir/outputs"

cc -x c - -x none "$super_object" -o "$tmp_dir/combine-super" <<'EOF'
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *rxcp_combine_superinstructions(const char *text);

char *mprintf(const char *format, ...) {
    va_list ap, copy;
    int length;
    char *result;
    va_start(ap, format);
    va_copy(copy, ap);
    length = vsnprintf(NULL, 0, format, copy);
    va_end(copy);
    if (length < 0) { va_end(ap); return NULL; }
    result = malloc((size_t)length + 1);
    if (result) vsnprintf(result, (size_t)length + 1, format, ap);
    va_end(ap);
    return result;
}

int main(int argc, char **argv) {
    FILE *input_file;
    long length;
    char *input;
    char *output;
    if (argc != 2 || !(input_file = fopen(argv[1], "rb"))) return 2;
    if (fseek(input_file, 0, SEEK_END) ||
        (length = ftell(input_file)) < 0 ||
        fseek(input_file, 0, SEEK_SET)) return 3;
    input = malloc((size_t)length + 1);
    if (!input || fread(input, 1, (size_t)length, input_file) != (size_t)length) return 4;
    fclose(input_file);
    input[length] = 0;
    output = rxcp_combine_superinstructions(input);
    free(input);
    if (!output) return 5;
    fputs(output, stdout);
    free(output);
    return 0;
}
EOF

tail -n +2 "$manifest" | while IFS=$'\t' read -r id workload mode image args expect; do
  source_name=${image%.rxbin}.rxas
  baseline="$baseline_dir/$source_name"
  transformed="$tmp_dir/transformed/$source_name"
  image_path="$tmp_dir/rxbin/$image"
  disassembly="$tmp_dir/disassembly/$source_name"
  profile="$tmp_dir/profiles/$id.csv"
  output="$tmp_dir/outputs/$id.txt"
  [[ -f "$baseline" ]] || { print -u2 -- "missing baseline RXAS: $baseline"; exit 1; }
  "$tmp_dir/combine-super" "$baseline" > "$transformed"
  "$rxas" -o "${image_path%.rxbin}" "$transformed" >/dev/null
  "$rxdas" -o "$disassembly" "$image_path" >/dev/null
  "$profile_vm" --profile-output "$profile" "$image_path" "$accepted_library" -a ${=args} > "$output" 2>&1
  rg -Fq "$expect" "$output" || { print -u2 -- "missing PASS marker for $id"; exit 1; }
done

awk '
  function hex(c) { c=tolower(c); return index("0123456789abcdef",c)-1 }
  function h2d(s,n,i) { n=0; for(i=1;i<=length(s);i++) n=n*16+hex(substr(s,i,1)); return n }
  FILENAME==ARGV[1] && /^X\(/ {
    line=$0; sub(/^X\(/,"",line); split(line,field,/,[[:space:]]*/)
    by_id[field[2]+0]=field[1]; next
  }
  FILENAME!=ARGV[1] && /\* 0x[0-9a-f]+:[0-9a-f]+/ {
    line=$0; sub(/^.*\* 0x[0-9a-f]+:/,"",line); sub(/[^0-9a-f].*$/,"",line)
    id=h2d(line); if(by_id[id]!="") count[by_id[id]]++
  }
  END { for(opcode in count) print opcode "\t" count[opcode] }
' "$rxops" "$tmp_dir"/disassembly/*.rxas | sort > "$tmp_dir/static.tsv"

awk -F, 'FNR>1 && $1=="instruction" { count[$2]+=$5 }
           END { for(opcode in count) print opcode "\t" count[opcode] }' \
  "$tmp_dir"/profiles/*.csv | sort > "$tmp_dir/dynamic.tsv"

awk -F'\t' '
  BEGIN { OFS="\t" }
  FILENAME==ARGV[1] && FNR>1 { form[++n]=$1; class[$1]=$2; family[$1]=$3; next }
  FILENAME==ARGV[2] { static[$1]=$2; next }
  FILENAME==ARGV[3] { dynamic[$1]=$2; next }
  END {
    print "opcode","class","family","replay_program_sites","replay_executions"
    for(i=1;i<=n;i++) print form[i],class[form[i]],family[form[i]],static[form[i]]+0,dynamic[form[i]]+0
  }
' "$form_manifest" "$tmp_dir/static.tsv" "$tmp_dir/dynamic.tsv" > "$script_dir/prebatch-replay.tsv"

{
  print -- "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print -- "repo_head=$(git -C "$repo_root" rev-parse HEAD)"
  print -- "host=$(uname -srm)"
  print -- "super_source_sha256=$(shasum -a 256 "$repo_root/compiler/rxcp_emit_super.c" | awk '{print $1}')"
  print -- "super_object_sha256=$(shasum -a 256 "$super_object" | awk '{print $1}')"
  print -- "rxas_sha256=$(shasum -a 256 "$rxas" | awk '{print $1}')"
  print -- "profile_vm_sha256=$(shasum -a 256 "$profile_vm" | awk '{print $1}')"
  print -- "accepted_library_sha256=$(shasum -a 256 "$accepted_library" | awk '{print $1}')"
  print -- "input_boundary=retained accepted pre-batch rxc RXAS for the 22 program images"
  print -- "transform_boundary=current rxc final Class 2 combiner followed by current optimizing rxas"
  print -- "library_boundary=accepted pre-batch linked library is executed unchanged and excluded from replay static sites"
  print -- "dynamic_boundary=bounded portfolio arguments from balanced-review/portfolio-manifest.tsv"
} > "$script_dir/prebatch-replay-provenance.txt"

print -- "$script_dir/prebatch-replay.tsv"
