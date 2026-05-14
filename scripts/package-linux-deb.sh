#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: scripts/package-linux-deb.sh --payload-dir DIR --output FILE [options]

Build a Debian package from a staged CREXX Linux payload directory. The payload
directory should contain the same files used for the portable ZIP, with bin/,
examples/, VERSION, BUILDINFO, and documentation at its root.

Options:
  --payload-dir DIR          Staged CREXX payload directory.
  --output FILE             Output .deb path.
  --package-name NAME       Debian package name. Defaults to crexx.
  --architecture ARCH       Debian architecture. Defaults to dpkg architecture.
  --install-root PATH       Install root. Defaults to /opt/crexx.
  --display-version TEXT    CREXX display version. Defaults to payload VERSION.
  --maintainer TEXT         Debian maintainer field.
  -h, --help                Show this help.
EOF
}

die() {
  echo "error: $*" >&2
  exit 1
}

payload_dir=""
output=""
package_name="crexx"
architecture=""
install_root="/opt/crexx"
display_version=""
maintainer="CREXX Project <noreply@github.com>"

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
    --package-name)
      [[ $# -ge 2 ]] || die "$1 requires NAME"
      package_name="$2"
      shift 2
      ;;
    --architecture)
      [[ $# -ge 2 ]] || die "$1 requires ARCH"
      architecture="$2"
      shift 2
      ;;
    --install-root)
      [[ $# -ge 2 ]] || die "$1 requires PATH"
      install_root="$2"
      shift 2
      ;;
    --display-version)
      [[ $# -ge 2 ]] || die "$1 requires TEXT"
      display_version="$2"
      shift 2
      ;;
    --maintainer)
      [[ $# -ge 2 ]] || die "$1 requires TEXT"
      maintainer="$2"
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

for cmd in awk basename chmod cp dpkg dpkg-deb du find install ln mkdir sed sort tr; do
  command -v "$cmd" >/dev/null || die "missing command: $cmd"
done

if [[ -z "$architecture" ]]; then
  architecture="$(dpkg --print-architecture)"
fi

if [[ -z "$display_version" ]]; then
  display_version="$(tr -d '[:space:]' < "$payload_dir/VERSION")"
fi

deb_version="${display_version#crexx-}"
deb_version="$(printf '%s' "$deb_version" | tr '[:upper:]' '[:lower:]')"
deb_version="${deb_version}-1"

if [[ ! "$package_name" =~ ^[a-z0-9][a-z0-9+.-]+$ ]]; then
  die "package name is not a valid Debian package name: $package_name"
fi

if [[ ! "$deb_version" =~ ^[0-9][A-Za-z0-9.+:~-]*-[A-Za-z0-9+.~]+$ ]]; then
  die "generated Debian version is not valid: $deb_version"
fi

work="$(mktemp -d "${TMPDIR:-/tmp}/crexx-deb.XXXXXX")"
trap "rm -rf -- $(printf '%q' "$work")" EXIT

pkgroot="$work/root"
package_root="${install_root#/}"
install -d "$pkgroot/$package_root" "$pkgroot/DEBIAN" "$pkgroot/usr/bin"

cp -a "$payload_dir"/. "$pkgroot/$package_root/"

while IFS= read -r tool_path; do
  tool="$(basename "$tool_path")"
  case "$tool" in
    *.a|*.dll|*.dylib|*.so|*.so.*|*.rxplugin|*.rxvmplugin|rxas_test_*)
      continue
      ;;
  esac
  ln -s "$install_root/bin/$tool" "$pkgroot/usr/bin/$tool"
done < <(find "$pkgroot/$package_root/bin" -maxdepth 1 -type f -perm -111 | sort)

installed_size="$(du -sk "$pkgroot/$package_root" "$pkgroot/usr/bin" | awk '{ total += $1 } END { print total }')"

cat > "$pkgroot/DEBIAN/control" <<EOF
Package: $package_name
Version: $deb_version
Section: devel
Priority: optional
Architecture: $architecture
Maintainer: $maintainer
Installed-Size: $installed_size
Homepage: https://github.com/adesutherland/CREXX
Description: cREXX toolchain and runtime
 CREXX is a custom REXX-to-bytecode toolchain. This package installs the
 compiler, assembler, linker, virtual machines, debugger, runtime files,
 examples, VERSION, and BUILDINFO under $install_root, with command symlinks
 in /usr/bin.
EOF

chmod 0755 "$pkgroot/DEBIAN"
chmod 0644 "$pkgroot/DEBIAN/control"

mkdir -p "$(dirname "$output")"
dpkg-deb --build --root-owner-group "$pkgroot" "$output"
dpkg-deb --info "$output"
