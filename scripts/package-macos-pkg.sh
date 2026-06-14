#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: scripts/package-macos-pkg.sh --payload-dir DIR --output FILE [options]

Build a macOS flat installer package from a staged CREXX macOS payload
directory. The payload directory should contain the same files used for the
portable ZIP, with bin/, examples/, VERSION, BUILDINFO, and documentation at
its root.

Options:
  --payload-dir DIR          Staged CREXX payload directory.
  --output FILE             Output .pkg path.
  --package-identifier ID   Package identifier. Defaults to org.crexx.crexx.
  --package-version TEXT    Package version. Defaults to payload VERSION.
  --install-root PATH       Install root. Defaults to /usr/local/crexx.
  --command-link-dir PATH   Command symlink dir. Defaults to /usr/local/bin.
  --sign-identity TEXT      Developer ID Installer identity.
  --keychain PATH           Keychain containing the signing identity.
  -h, --help                Show this help.
EOF
}

die() {
  echo "error: $*" >&2
  exit 1
}

payload_dir=""
output=""
package_identifier="org.crexx.crexx"
package_version=""
install_root="/usr/local/crexx"
command_link_dir="/usr/local/bin"
sign_identity=""
keychain=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --payload-dir)
      [[ $# -ge 2 ]] || die "$1 requires DIR"
      payload_dir="$2"
      shift 2
      ;;
    --output)
      [[ $# -ge 2 ]] || die "$1 requires FILE"
      output="$2"
      shift 2
      ;;
    --package-identifier)
      [[ $# -ge 2 ]] || die "$1 requires ID"
      package_identifier="$2"
      shift 2
      ;;
    --package-version)
      [[ $# -ge 2 ]] || die "$1 requires TEXT"
      package_version="$2"
      shift 2
      ;;
    --install-root)
      [[ $# -ge 2 ]] || die "$1 requires PATH"
      install_root="$2"
      shift 2
      ;;
    --command-link-dir)
      [[ $# -ge 2 ]] || die "$1 requires PATH"
      command_link_dir="$2"
      shift 2
      ;;
    --sign-identity)
      [[ $# -ge 2 ]] || die "$1 requires TEXT"
      sign_identity="$2"
      shift 2
      ;;
    --keychain)
      [[ $# -ge 2 ]] || die "$1 requires PATH"
      keychain="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      die "unknown option: $1"
      ;;
  esac
done

[[ -n "$payload_dir" ]] || die "--payload-dir is required"
[[ -n "$output" ]] || die "--output is required"
[[ -d "$payload_dir" ]] || die "payload directory not found: $payload_dir"
[[ -d "$payload_dir/bin" ]] || die "payload directory does not contain bin/: $payload_dir"
[[ -f "$payload_dir/VERSION" ]] || die "payload directory does not contain VERSION: $payload_dir"

for cmd in basename cp find install ln mkdir pkgbuild sort; do
  command -v "$cmd" >/dev/null || die "missing command: $cmd"
done

if [[ -z "$package_version" ]]; then
  package_version="$(tr -d '[:space:]' < "$payload_dir/VERSION")"
fi
package_version="${package_version#crexx-}"

[[ "$install_root" = /* ]] || die "--install-root must be absolute: $install_root"
[[ "$command_link_dir" = /* ]] || die "--command-link-dir must be absolute: $command_link_dir"
[[ "$package_identifier" =~ ^[A-Za-z0-9][A-Za-z0-9.-]*$ ]] ||
  die "package identifier is not valid: $package_identifier"
[[ "$package_version" =~ ^[A-Za-z0-9][A-Za-z0-9.+:~-]*$ ]] ||
  die "package version is not valid: $package_version"

work="$(mktemp -d "${TMPDIR:-/tmp}/crexx-macos-pkg.XXXXXX")"
trap "rm -rf -- $(printf '%q' "$work")" EXIT

pkgroot="$work/root"
package_root="${install_root#/}"
command_root="${command_link_dir#/}"

install -d "$pkgroot/$package_root" "$pkgroot/$command_root"
cp -a "$payload_dir"/. "$pkgroot/$package_root/"

while IFS= read -r tool_path; do
  tool="$(basename "$tool_path")"
  case "$tool" in
    *.a|*.dylib|*.so|*.so.*|*.rxplugin|*.rxvmplugin|rxas_test_*)
      continue
      ;;
  esac
  ln -s "$install_root/bin/$tool" "$pkgroot/$command_root/$tool"
done < <(find "$pkgroot/$package_root/bin" -maxdepth 1 -type f -perm -111 | sort)

pkgbuild_args=(
  --root "$pkgroot"
  --identifier "$package_identifier"
  --version "$package_version"
  --install-location /
  --ownership recommended
)

if [[ -n "$sign_identity" ]]; then
  pkgbuild_args+=(--sign "$sign_identity" --timestamp)
  if [[ -n "$keychain" ]]; then
    pkgbuild_args+=(--keychain "$keychain")
  fi
fi

mkdir -p "$(dirname "$output")"
pkgbuild "${pkgbuild_args[@]}" "$output"
