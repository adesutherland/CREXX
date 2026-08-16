#!/usr/bin/env bash

set -euo pipefail

usage() {
  printf '%s\n' \
    "usage: $0 EXPECTED_FULL_COMMIT BUILD_DIRECTORY EVIDENCE_DIRECTORY"
}

die() {
  printf 'ERROR: %s\n' "$*" >&2
  exit 1
}

[[ $# -eq 3 ]] || {
  usage
  exit 2
}

expected_commit=$1
build_argument=$2
evidence_argument=$3

[[ "$expected_commit" =~ ^[0-9a-f]{40}$ ]] ||
  die "expected commit must be a full lowercase 40-character SHA"
[[ "$(uname -s)" == Linux ]] || die "this runner requires Linux"

for required_tool in git cmake ctest ninja zip dpkg dpkg-deb realpath \
  sha256sum cc awk find sort xargs grep cut tee cp tr; do
  command -v "$required_tool" >/dev/null ||
    die "required tool is unavailable: $required_tool"
done

repository_root=$(git rev-parse --show-toplevel)
[[ "$(git rev-parse HEAD)" == "$expected_commit" ]] ||
  die "HEAD does not match expected commit $expected_commit"
[[ -z "$(git status --porcelain --untracked-files=all)" ]] ||
  die "qualification checkout is not clean"

build_directory=$(realpath -m "$build_argument")
evidence_directory=$(realpath -m "$evidence_argument")
repository_root=$(cd "$repository_root" && pwd -P)
artifact_directory="${evidence_directory}-artifacts"

[[ "$build_directory" != "$evidence_directory" ]] ||
  die "build and evidence directories must differ"
[[ "$build_directory" != "$artifact_directory" ]] ||
  die "build and artifact directories must differ"
case "$build_directory/" in
  "$repository_root/"*) die "build directory must be outside the repository" ;;
esac
case "$evidence_directory/" in
  "$repository_root/"*) die "evidence directory must be outside the repository" ;;
esac
case "$artifact_directory/" in
  "$repository_root/"*) die "artifact directory must be outside the repository" ;;
esac
mkdir -p "$build_directory" "$evidence_directory"
[[ -z "$(find "$build_directory" -mindepth 1 -print -quit)" ]] ||
  die "build directory is not empty"
[[ -z "$(find "$evidence_directory" -mindepth 1 -print -quit)" ]] ||
  die "evidence directory is not empty"
if [[ -e "$artifact_directory" ]]; then
  [[ -d "$artifact_directory" ]] || die "artifact path is not a directory"
  [[ -z "$(find "$artifact_directory" -mindepth 1 -print -quit)" ]] ||
    die "artifact directory is not empty"
else
  mkdir -p "$artifact_directory"
fi

build_jobs=${CREXX_QUALIFICATION_BUILD_JOBS:-4}
test_jobs=${CREXX_QUALIFICATION_TEST_JOBS:-10}
[[ "$build_jobs" =~ ^[1-9][0-9]*$ ]] || die "invalid build job count"
[[ "$test_jobs" =~ ^[1-9][0-9]*$ ]] || die "invalid test job count"

mkdir -p "$evidence_directory/labels" "$evidence_directory/install" \
  "$evidence_directory/package"
exec > >(tee "$evidence_directory/qualification.log") 2>&1

on_error() {
  failure_code=$?
  printf 'FAIL: Linux qualification stopped with exit code %s\n' "$failure_code"
  exit "$failure_code"
}
trap on_error ERR

export CREXX_HTTP_TLS_LIVE_VERIFY=1
export CREXX_HTTP_TLS_LIVE_HOST=${CREXX_HTTP_TLS_LIVE_HOST:-example.com}
export CREXX_HTTP_TLS_MISMATCH_HOST=${CREXX_HTTP_TLS_MISMATCH_HOST:-wrong.host.badssl.com}

{
  printf 'expected_commit=%s\n' "$expected_commit"
  printf 'head=%s\n' "$(git -C "$repository_root" rev-parse HEAD)"
  printf 'branch=%s\n' "$(git -C "$repository_root" branch --show-current)"
  printf 'uname=%s\n' "$(uname -srm)"
  printf 'compiler=%s\n' "$(cc --version | head -1)"
  printf 'cmake=%s\n' "$(cmake --version | head -1)"
  printf 'ninja=%s\n' "$(ninja --version)"
  printf 'build_jobs=%s\n' "$build_jobs"
  printf 'test_jobs=%s\n' "$test_jobs"
  printf 'tls_trusted_host=%s\n' "$CREXX_HTTP_TLS_LIVE_HOST"
  printf 'tls_mismatch_host=%s\n' "$CREXX_HTTP_TLS_MISMATCH_HOST"
} > "$evidence_directory/provenance.txt"

cmake -S "$repository_root" -B "$build_directory" -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCREXX_CONCURRENCY_CTEST_JOBS="$test_jobs" \
  2>&1 | tee "$evidence_directory/configure.log"

cmake --build "$build_directory" --parallel "$build_jobs" \
  2>&1 | tee "$evidence_directory/build.log"

cmake --build "$build_directory" --target concurrency-qa \
  --parallel "$build_jobs" \
  2>&1 | tee "$evidence_directory/concurrency-matrix.log"

ctest --test-dir "$build_directory" --parallel 1 --output-on-failure -V \
  -R '^ts_http_tls_live_' \
  2>&1 | tee "$evidence_directory/tls-live.log"

ctest --test-dir "$build_directory" --parallel "$test_jobs" \
  --output-on-failure --repeat until-fail:20 \
  -L '^concurrency-stress$' \
  2>&1 | tee "$evidence_directory/stress-repeat-20.log"

ctest --test-dir "$build_directory" --parallel "$test_jobs" \
  --output-on-failure --no-tests=error \
  2>&1 | tee "$evidence_directory/full-ctest.log"

ctest --test-dir "$build_directory" -N -L '^concurrency$' \
  > "$evidence_directory/labels/concurrency.txt"
umbrella_count=$(awk '/Total Tests:/ {print $3}' \
  "$evidence_directory/labels/concurrency.txt")
[[ "$umbrella_count" =~ ^[1-9][0-9]*$ ]] ||
  die "concurrency umbrella label selected no tests"
for solution_number in 01 02 03 04 05 06 07 08 09; do
  label="concurrency-sp${solution_number}"
  label_file="$evidence_directory/labels/${label}.txt"
  ctest --test-dir "$build_directory" -N -L "^${label}$" > "$label_file"
  label_count=$(awk '/Total Tests:/ {print $3}' "$label_file")
  [[ "$label_count" =~ ^[1-9][0-9]*$ ]] ||
    die "$label selected no tests"
done

install_prefix="$evidence_directory/install/prefix"
cmake --install "$build_directory" --prefix "$install_prefix" \
  2>&1 | tee "$evidence_directory/install/install.log"
find "$install_prefix" -type f -print | sort \
  > "$evidence_directory/install/inventory.txt"

run_toolchain_smoke() {
  smoke_name=$1
  bin_directory=$2
  smoke_directory="$evidence_directory/$smoke_name"
  mkdir -p "$smoke_directory"
  cp "$repository_root/docs/books/crexx_programming_guide/examples/concurrency_basic.crexx" \
    "$smoke_directory/"
  (
    cd "$smoke_directory"
    "$bin_directory/rxc" -i "$bin_directory" -x \
      -o concurrency_basic concurrency_basic.crexx
    "$bin_directory/rxas" -o concurrency_basic concurrency_basic
    "$bin_directory/rxlink" -s -o concurrency_basic_linked \
      concurrency_basic.rxbin "$bin_directory/library" \
      "$bin_directory/classlib"
    "$bin_directory/rxbvm" concurrency_basic_linked.rxbin
    if [[ -x "$bin_directory/rxtvm" ]]; then
      "$bin_directory/rxtvm" concurrency_basic_linked.rxbin
    fi
  ) 2>&1 | tee "$evidence_directory/${smoke_name}.log"
}

run_toolchain_smoke install/smoke "$install_prefix/bin"
install_vm_count=1
if [[ -x "$install_prefix/bin/rxtvm" ]]; then
  install_vm_count=2
fi
[[ "$(grep -c '^PASS: basic concurrency example$' \
  "$evidence_directory/install/smoke.log")" -eq "$install_vm_count" ]] ||
  die "installed toolchain smoke did not report PASS for every VM"

payload_root="$artifact_directory/payload"
payload_name=CREXX-linux-"$(uname -m)"
payload_directory="$payload_root/$payload_name"
mkdir -p "$payload_directory/bin" "$payload_directory/examples"
cp -R "$build_directory/bin/." "$payload_directory/bin/"
cp -R "$repository_root/examples/." "$payload_directory/examples/"
cp -R "$build_directory/example-artifacts/." "$payload_directory/examples/"
cp "$repository_root/LICENSE" "$repository_root/README.md" \
  "$repository_root/SECURITY.md" "$repository_root/INSTALL-RUN.md" \
  "$payload_directory/"
cp "$build_directory/generated/VERSION" "$payload_directory/VERSION"
cp "$build_directory/generated/BUILDINFO" "$payload_directory/BUILDINFO"

archive="$artifact_directory/${payload_name}.zip"
(cd "$payload_root" && zip -qr "$archive" "$payload_name")
zip -T "$archive"

deb_package="$artifact_directory/${payload_name}.deb"
"$repository_root/scripts/package-linux-deb.sh" \
  --payload-dir "$payload_directory" --output "$deb_package" \
  --display-version "$(tr -d '[:space:]' < "$payload_directory/VERSION")" \
  2>&1 | tee "$evidence_directory/package/deb-build.log"

deb_extract="$artifact_directory/deb-extract"
mkdir -p "$deb_extract"
dpkg-deb -x "$deb_package" "$deb_extract"
run_toolchain_smoke package/deb-smoke "$deb_extract/opt/crexx/bin"
package_vm_count=1
if [[ -x "$deb_extract/opt/crexx/bin/rxtvm" ]]; then
  package_vm_count=2
fi
[[ "$(grep -c '^PASS: basic concurrency example$' \
  "$evidence_directory/package/deb-smoke.log")" -eq "$package_vm_count" ]] ||
  die "extracted Debian package smoke did not report PASS for every VM"

for required_payload in library.rxbin classlib.rxbin rxfnsg.rxbin; do
  [[ -f "$payload_directory/bin/$required_payload" ]] ||
    die "portable package is missing $required_payload"
done
find "$payload_directory" -type f -print | sort \
  > "$evidence_directory/package/inventory.txt"
(
  cd "$artifact_directory"
  sha256sum "$(basename "$archive")" "$(basename "$deb_package")"
) > "$evidence_directory/package/packages.sha256"

grep -E '^(CMAKE_BUILD_TYPE|CREXX_ENABLE_TLS|CREXX_VM_HANDLER_PANEL|CREXX_VM_PROFILING):' \
  "$build_directory/CMakeCache.txt" >> "$evidence_directory/provenance.txt"
[[ -z "$(git -C "$repository_root" status --porcelain --untracked-files=all)" ]] ||
  die "qualification changed the checkout"

{
  printf 'PASS: initial concurrency Linux qualification\n'
  printf 'commit=%s\n' "$expected_commit"
  printf 'concurrency_tests=%s\n' "$umbrella_count"
  printf 'tls_backend=%s\n' \
    "$(grep '^CREXX_ENABLE_TLS:STRING=' "$build_directory/CMakeCache.txt" | cut -d= -f2-)"
  printf 'portable_archive=%s\n' "$(basename "$archive")"
  printf 'debian_package=%s\n' "$(basename "$deb_package")"
  printf 'artifact_directory=%s\n' "$artifact_directory"
} > "$evidence_directory/RESULT.txt"

(
  cd "$evidence_directory"
  find . -type f ! -name SHA256SUMS ! -name qualification.log -print0 |
    sort -z | xargs -0 sha256sum > SHA256SUMS
)

trap - ERR
printf 'PASS: Linux qualification evidence retained at %s\n' \
  "$evidence_directory"
