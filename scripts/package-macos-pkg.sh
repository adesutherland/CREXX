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
  --background-image PATH   Installer background image. Defaults to
                            packaging/macos/assets/crexx-installer-background.png.
  --no-background-image     Do not include a custom Installer background.
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
background_image=""
use_background_image=1
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
    --background-image)
      [[ $# -ge 2 ]] || die "$1 requires PATH"
      background_image="$2"
      use_background_image=1
      shift 2
      ;;
    --no-background-image)
      use_background_image=0
      shift
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

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/.." && pwd)"

if [[ -z "$background_image" ]]; then
  background_image="$repo_root/packaging/macos/assets/crexx-installer-background.png"
fi

for cmd in basename cp ditto find install ln mkdir pkgbuild productbuild sort; do
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

if [[ "$use_background_image" -eq 1 ]]; then
  [[ -f "$background_image" ]] || die "background image not found: $background_image"
  background_image="$(cd "$(dirname "$background_image")" && pwd)/$(basename "$background_image")"
fi

work="$(mktemp -d "${TMPDIR:-/tmp}/crexx-macos-pkg.XXXXXX")"
trap "rm -rf -- $(printf '%q' "$work")" EXIT

pkgroot="$work/root"
package_root="${install_root#/}"
command_root="${command_link_dir#/}"

install -d "$pkgroot/$package_root" "$pkgroot/$command_root"
ditto --norsrc "$payload_dir" "$pkgroot/$package_root"

while IFS= read -r tool_path; do
  tool="$(basename "$tool_path")"
  case "$tool" in
    *.a|*.dylib|*.so|*.so.*|*.rxplugin|*.rxvmplugin|rxas_test_*)
      continue
      ;;
  esac
  ln -s "$install_root/bin/$tool" "$pkgroot/$command_root/$tool"
done < <(find "$pkgroot/$package_root/bin" -maxdepth 1 -type f -perm -111 | sort)

if command -v xattr >/dev/null; then
  xattr -crs "$pkgroot"
fi
if command -v dot_clean >/dev/null; then
  dot_clean -m "$pkgroot"
fi

component_pkg="$work/crexx-component.pkg"
distribution="$work/Distribution"
resources="$work/resources"

pkgbuild_args=(
  --root "$pkgroot"
  --identifier "$package_identifier"
  --version "$package_version"
  --install-location /
  --ownership recommended
  --filter '(^|/)\._[^/]*$'
  --filter '(^|/)\.DS_Store$'
  --filter '(^|/)CVS($|/)'
  --filter '(^|/)\.svn($|/)'
)

COPYFILE_DISABLE=1 pkgbuild "${pkgbuild_args[@]}" "$component_pkg"

mkdir -p "$resources"

if [[ "$use_background_image" -eq 1 ]]; then
  cp "$background_image" "$resources/crexx-installer-background.png"
fi

cat > "$resources/Welcome.html" <<EOF
<!doctype html>
<html>
<head>
<style>
html,
body {
  background: #f8f2e7;
  color: #111111;
  font-family: -apple-system, BlinkMacSystemFont, "Helvetica Neue", sans-serif;
}
body {
  margin: 14px 18px;
}
h1 {
  font-size: 24px;
  line-height: 1.1;
  margin: 0 0 20px;
}
p {
  font-size: 13px;
  line-height: 1.35;
}
code {
  font-family: ui-monospace, SFMono-Regular, Menlo, monospace;
}
</style>
</head>
<body>
<h1>Install CREXX ${package_version}</h1>
<p>This package installs the CREXX command-line toolchain and runtime under <code>${install_root}</code>.</p>
<p>It also creates command links in <code>${command_link_dir}</code> so new terminal sessions can run <code>crexx</code>, <code>rxc</code>, <code>rxas</code>, <code>rxlink</code>, and <code>rxvm</code> from PATH.</p>
</body>
</html>
EOF

cat > "$resources/Conclusion.html" <<EOF
<!doctype html>
<html>
<head>
<style>
html,
body {
  background: #f8f2e7;
  color: #111111;
  font-family: -apple-system, BlinkMacSystemFont, "Helvetica Neue", sans-serif;
}
body {
  margin: 14px 18px;
}
h1 {
  font-size: 24px;
  line-height: 1.1;
  margin: 0 0 20px;
}
p {
  font-size: 13px;
  line-height: 1.35;
}
code {
  font-family: ui-monospace, SFMono-Regular, Menlo, monospace;
}
</style>
</head>
<body>
<h1>CREXX is installed</h1>
<p>Open a new terminal and run <code>crexx ${install_root}/examples/hello.crexx</code> to check the installation.</p>
</body>
</html>
EOF

cat > "$distribution" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>CREXX ${package_version}</title>
EOF

if [[ "$use_background_image" -eq 1 ]]; then
  cat >> "$distribution" <<'EOF'
    <background file="crexx-installer-background.png" mime-type="image/png" alignment="left" scaling="proportional"/>
    <background-darkAqua file="crexx-installer-background.png" mime-type="image/png" alignment="left" scaling="proportional"/>
EOF
fi

cat >> "$distribution" <<EOF
    <welcome file="Welcome.html" mime-type="text/html"/>
    <conclusion file="Conclusion.html" mime-type="text/html"/>
    <pkg-ref id="${package_identifier}"/>
    <options customize="never" require-scripts="false" hostArchitectures="x86_64,arm64"/>
    <choices-outline>
        <line choice="default">
            <line choice="${package_identifier}"/>
        </line>
    </choices-outline>
    <choice id="default"/>
    <choice id="${package_identifier}" visible="false">
        <pkg-ref id="${package_identifier}"/>
    </choice>
    <pkg-ref id="${package_identifier}" version="${package_version}" onConclusion="none">crexx-component.pkg</pkg-ref>
</installer-gui-script>
EOF

productbuild_args=(
  --distribution "$distribution"
  --package-path "$work"
  --resources "$resources"
)

if [[ -n "$sign_identity" ]]; then
  productbuild_args+=(--sign "$sign_identity" --timestamp)
  if [[ -n "$keychain" ]]; then
    productbuild_args+=(--keychain "$keychain")
  fi
fi

mkdir -p "$(dirname "$output")"
COPYFILE_DISABLE=1 productbuild "${productbuild_args[@]}" "$output"
