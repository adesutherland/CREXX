#!/usr/bin/env bash
# Controlled commands only: this test never changes the host packet filter.
set -euo pipefail
script_dir="$(cd "$(dirname "$0")" && pwd)"
work="$(mktemp -d "${TMPDIR:-/tmp}/crexx-stapling-network.XXXXXX")"
mkdir "$work/bin"
cleanup() {
  local item
  for item in "$work"/bin/* "$work"/calls "$work"/rule "$work"/output; do
    if [[ -f "$item" ]]; then
      unlink "$item"
    fi
  done
  rmdir "$work/bin" "$work"
}
trap cleanup EXIT
cat > "$work/bin/uname" <<'SH'
#!/usr/bin/env bash
case "$1" in
  -s) printf '%s\n' "${FIXTURE_OS:-Darwin}" ;;
  -m) printf '%s\n' "${FIXTURE_ARCH:-x86_64}" ;;
  *) exit 91 ;;
esac
SH
cat > "$work/bin/sudo" <<'SH'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$FIXTURE_WORK/calls"
if [[ "$*" == 'pfctl -f -' ]]; then
  cat > "$FIXTURE_WORK/rule"
  exit "${FIXTURE_LOAD_RESULT:-0}"
elif [[ "$*" == 'pfctl -E' ]]; then
  exit "${FIXTURE_ENABLE_RESULT:-0}"
fi
exit 92
SH
chmod +x "$work/bin/uname" "$work/bin/sudo"
export PATH="$work/bin:$PATH" FIXTURE_WORK="$work"
export GITHUB_ACTIONS=true RUNNER_ENVIRONMENT=github-hosted
export FIXTURE_OS=Darwin FIXTURE_ARCH=x86_64
export FIXTURE_LOAD_RESULT=0 FIXTURE_ENABLE_RESULT=0

expect_status() {
  local expected="$1" result=0
  : > "$work/calls"
  bash "$script_dir/prepare-macos-stapling-network.sh" > "$work/output" 2>&1 || result=$?
  [[ "$result" == "$expected" ]] || { cat "$work/output"; exit 1; }
}
expect_status 0
[[ "$(cat "$work/rule")" == 'block return out quick proto udp from any to any port 443' ]]
[[ "$(cat "$work/calls")" == $'pfctl -f -\npfctl -E' ]]

for setting in actions self-hosted os architecture; do
  case "$setting" in
    actions) GITHUB_ACTIONS=false ;;
    self-hosted) RUNNER_ENVIRONMENT=self-hosted ;;
    os) FIXTURE_OS=Linux ;;
    architecture) FIXTURE_ARCH=arm64 ;;
  esac
  expect_status 64
  [[ ! -s "$work/calls" ]]
  GITHUB_ACTIONS=true RUNNER_ENVIRONMENT=github-hosted FIXTURE_OS=Darwin FIXTURE_ARCH=x86_64
done
FIXTURE_LOAD_RESULT=17
expect_status 17
[[ "$(cat "$work/calls")" == 'pfctl -f -' ]]
FIXTURE_LOAD_RESULT=0 FIXTURE_ENABLE_RESULT=19
expect_status 19
[[ "$(cat "$work/calls")" == $'pfctl -f -\npfctl -E' ]]
printf '%s\n' 'PASS: Intel hosted stapling transport gates and failure propagation'
