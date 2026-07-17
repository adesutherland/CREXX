#!/bin/zsh

set -euo pipefail

if (( $# != 1 )); then
  print -u2 "usage: $0 CANDIDATE_DIR"
  exit 2
fi

repo_root=${0:a:h:h:h:h}
candidate_dir=${1:a}
build_dir="$repo_root/cmake-build-release"
evidence_dir=${0:a:h}

{
  print "collected_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  print "repo_root=$repo_root"
  print "branch=$(git -C "$repo_root" branch --show-current)"
  print "head=$(git -C "$repo_root" rev-parse HEAD)"
  print "origin_develop=$(git -C "$repo_root" rev-parse origin/develop)"
  print "uname=$(uname -a)"
  print "cpu=$(sysctl -n machdep.cpu.brand_string)"
  print "logical_cpus=$(sysctl -n hw.logicalcpu)"
  sw_vers
  cmake --version | sed -n '1p'
  ninja --version | sed 's/^/ninja /'
  clang --version | sed -n '1p'
  rg '^(CMAKE_BUILD_TYPE|CREXX_VM_PROFILING|CMAKE_GENERATOR):' "$build_dir/CMakeCache.txt"
  print 'dirty_worktree:'
  git -C "$repo_root" status --short
} > "$evidence_dir/host-and-build.txt"

typeset -A artifact_path
artifact_path[A-canonical-source]="$repo_root/tests/benchmarks/rexxcps_levelb.crexx"
artifact_path[A-opaque-source]="$repo_root/tests/benchmarks/rexxcps_levelb_opaque.crexx"
artifact_path[A-canonical-noopt-rxas]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_noopt.rxas"
artifact_path[A-canonical-opt-rxas]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxas"
artifact_path[A-canonical-noopt-rxbin]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_noopt.rxbin"
artifact_path[A-canonical-opt-rxbin]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin"
artifact_path[A-opaque-noopt-rxas]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opaque_noopt.rxas"
artifact_path[A-opaque-opt-rxas]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opaque_opt.rxas"
artifact_path[A-opaque-noopt-rxbin]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opaque_noopt.rxbin"
artifact_path[A-opaque-opt-rxbin]="$build_dir/tests/benchmarks/benchmark_rexxcps_levelb_opaque_opt.rxbin"

for candidate in B C; do
  for variant in canonical opaque; do
    artifact_path[$candidate-$variant-source]="$candidate_dir/rexxcps_${candidate}_${variant}.crexx"
    for mode in noopt opt; do
      artifact_path[$candidate-$variant-$mode-rxas]="$candidate_dir/rexxcps_${candidate}_${variant}_${mode}.rxas"
      artifact_path[$candidate-$variant-$mode-rxbin]="$candidate_dir/rexxcps_${candidate}_${variant}_${mode}.rxbin"
    done
  done
done

print 'artifact,sha256' > "$evidence_dir/artifact-hashes.csv"
for artifact in ${(ok)artifact_path}; do
  digest=$(shasum -a 256 "${artifact_path[$artifact]}" | awk '{print $1}')
  print "$artifact,$digest" >> "$evidence_dir/artifact-hashes.csv"
done

{
  diff -U0 --label tests/benchmarks/rexxcps_levelb.crexx --label candidate-B/rexxcps_levelb.crexx \
    "$repo_root/tests/benchmarks/rexxcps_levelb.crexx" "$candidate_dir/rexxcps_B_canonical.crexx" || true
  diff -U0 --label tests/benchmarks/rexxcps_levelb_opaque.crexx --label candidate-B/rexxcps_levelb_opaque.crexx \
    "$repo_root/tests/benchmarks/rexxcps_levelb_opaque.crexx" "$candidate_dir/rexxcps_B_opaque.crexx" || true
  diff -U0 --label candidate-B/rexxcps_levelb.crexx --label candidate-C/rexxcps_levelb.crexx \
    "$candidate_dir/rexxcps_B_canonical.crexx" "$candidate_dir/rexxcps_C_canonical.crexx" || true
  diff -U0 --label candidate-B/rexxcps_levelb_opaque.crexx --label candidate-C/rexxcps_levelb_opaque.crexx \
    "$candidate_dir/rexxcps_B_opaque.crexx" "$candidate_dir/rexxcps_C_opaque.crexx" || true
} > "$evidence_dir/candidate-sources.patch"

print 'vm,candidate,n,cps_mean,cps_delta_vs_A_percent,cps_min,cps_max,wall_mean_s,wall_delta_vs_A_percent,wall_min_s,wall_max_s' > "$evidence_dir/summary.csv"
awk -F, '
  NR > 1 {
    key = $2 "," $5
    count[key]++
    cps[key] += $7
    wall[key] += $8
    if (count[key] == 1 || $7 < cps_min[key]) cps_min[key] = $7
    if (count[key] == 1 || $7 > cps_max[key]) cps_max[key] = $7
    if (count[key] == 1 || $8 < wall_min[key]) wall_min[key] = $8
    if (count[key] == 1 || $8 > wall_max[key]) wall_max[key] = $8
  }
  END {
    for (v = 1; v <= 2; v++) {
      vm = (v == 1 ? "rxvm" : "rxbvm")
      base_cps = cps[vm ",A"] / count[vm ",A"]
      base_wall = wall[vm ",A"] / count[vm ",A"]
      for (i = 1; i <= 3; i++) {
        candidate = (i == 1 ? "A" : (i == 2 ? "B" : "C"))
        key = vm "," candidate
        mean_cps = cps[key] / count[key]
        mean_wall = wall[key] / count[key]
        printf "%s,%s,%d,%.3f,%+.3f,%d,%d,%.3f,%+.3f,%.2f,%.2f\n", \
          vm, candidate, count[key], mean_cps, 100 * (mean_cps / base_cps - 1), \
          cps_min[key], cps_max[key], mean_wall, 100 * (mean_wall / base_wall - 1), \
          wall_min[key], wall_max[key]
      }
    }
  }
' "$evidence_dir/samples.csv" >> "$evidence_dir/summary.csv"

typeset -A rxas_path
rxas_path[A-canonical-noopt]="${artifact_path[A-canonical-noopt-rxas]}"
rxas_path[A-canonical-opt]="${artifact_path[A-canonical-opt-rxas]}"
rxas_path[A-opaque-noopt]="${artifact_path[A-opaque-noopt-rxas]}"
rxas_path[A-opaque-opt]="${artifact_path[A-opaque-opt-rxas]}"
rxas_path[B-canonical-noopt]="${artifact_path[B-canonical-noopt-rxas]}"
rxas_path[B-canonical-opt]="${artifact_path[B-canonical-opt-rxas]}"
rxas_path[B-opaque-noopt]="${artifact_path[B-opaque-noopt-rxas]}"
rxas_path[B-opaque-opt]="${artifact_path[B-opaque-opt-rxas]}"
rxas_path[C-canonical-noopt]="${artifact_path[C-canonical-noopt-rxas]}"
rxas_path[C-canonical-opt]="${artifact_path[C-canonical-opt-rxas]}"
rxas_path[C-opaque-noopt]="${artifact_path[C-opaque-noopt-rxas]}"
rxas_path[C-opaque-opt]="${artifact_path[C-opaque-opt-rxas]}"

ops=(dadd dsub dmult ddiv dgt dlt deq fadd fsub fmult fdiv fgt flt feq iadd isub imult idiv igt ilt ieq ftod itod stod dtos ftoi itof stof stoi ftos itos)
print 'artifact,opcode,count' > "$evidence_dir/rxas-opcounts.csv"
for artifact in ${(ok)rxas_path}; do
  for op in $ops; do
    count=$(awk -v op="$op" '$1 == op {count++} END {print count + 0}' "${rxas_path[$artifact]}")
    print "$artifact,$op,$count" >> "$evidence_dir/rxas-opcounts.csv"
  done
done

{
  for artifact in ${(ok)rxas_path}; do
    print "[$artifact workload and generated procedures]"
    awk '
      /^\/\* Imported Declaration/ {exit}
      /^[[:alnum:]_]+\(.*\) \.locals=/ {procedure = $1}
      $1 == "setnumdgts" {print procedure, $0}
    ' "${rxas_path[$artifact]}"
  done
} > "$evidence_dir/numeric-contexts.txt"

{
  for artifact in ${(ok)rxas_path}; do
    print "[$artifact numeric main metadata]"
    rg '^   \.meta ".*\.main\.[^"]+"="b" "\.(int|float|decimal)"' "${rxas_path[$artifact]}" || true
  done
} > "$evidence_dir/numeric-main-metadata.txt"

{
  for artifact in ${(ok)rxas_path}; do
    print "[$artifact relevant calls]"
    rg 'call .*rxfnsb\.(trunc|format|floattrunc|floatformat)\(\)' "${rxas_path[$artifact]}" || true
  done
} > "$evidence_dir/rxas-bif-calls.txt"

shasum -a 256 "$evidence_dir/samples.csv" "$evidence_dir/summary.csv" > "$evidence_dir/evidence-hashes.txt"
