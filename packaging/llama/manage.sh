#!/bin/sh
# OS-only installer helper. The .pkg carries these files in its signed scripts
# archive. Installed copy: <core>/.llama-installer/manage.sh remove <core>.
set -eu
here=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)
die() { echo "llama.rexx: $*" >&2; exit 1; }
action=${1:-check}
root=${2:-}
case "$action" in check|install|remove) ;; *) die 'Use check, install or remove [CREXX directory].';; esac

if [ -z "$root" ]; then
    candidates=$(mktemp)
    trap 'rm -f "$candidates"' EXIT HUP INT TERM
    candidate() {
        [ -f "$1/core-package.json" ] || return 0
        (CDPATH='' cd -- "$1" && pwd -P) >> "$candidates"
    }
    candidate /usr/local/crexx
    if [ -n "${CREXX_HOME:-}" ]; then candidate "$CREXX_HOME"; fi
    if [ -n "${REXX_HOME:-}" ]; then candidate "$REXX_HOME"; fi
    # The core pkg installs absolute paths with install-location=/; its receipt
    # remains useful when the packager chose a nondefault prefix.
    /usr/sbin/pkgutil --files org.crexx.crexx 2>/dev/null |
      while IFS= read -r name; do
          case "$name" in */core-package.json) candidate "/${name%/core-package.json}";; esac
      done
    count=$(sort -u "$candidates" | wc -l | tr -d ' ')
    [ "$count" = 1 ] || die 'No unique cREXX installation found. Install the matching core first, or run the supplied helper with an explicit cREXX directory.'
    root=$(sort -u "$candidates")
    rm -f "$candidates"
    trap - EXIT HUP INT TERM
fi
[ -d "$root" ] || die "cREXX directory does not exist: $root"
root=$(CDPATH='' cd -- "$root" && pwd -P)
state="$root/.llama-installer"
[ ! -L "$state" ] || die 'Installer state is a symlink.'
lock="$root/.llama-install-lock"
mkdir "$lock" 2>/dev/null || die 'Another plugin installation is running (or a previous interrupted installation left its lock).'
trap 'rmdir "$lock"' EXIT
trap 'exit 1' HUP INT TERM

safe_target() {
    name=$1
    case "$name" in ''|/*|../*|*/../*|*/..|./*|*/./*|*\\*) die "Unsafe file: $name";; esac
    walk="$root/$name"
    while [ "$walk" != "$root" ]; do
        [ ! -L "$walk" ] || die "Refusing plugin destination symlink: $walk"
        walk=${walk%/*}
    done
}
verify() {
    prefix=$1
    list=$2
    [ -f "$list" ] || die "Missing installer manifest: $list"
    (cd "$prefix" && /usr/bin/shasum -a 256 --check --status "$list") ||
        die "Missing, changed or incompatible files in $prefix"
}
targets() {
    while IFS= read -r line; do safe_target "${line#*  }"; done < "$1"
}
remove_files() {
    while IFS= read -r line; do rm -f -- "$root/${line#*  }"; done < "$1"
}
copy_files() {
    from=$1
    list=$2
    to=$3
    while IFS= read -r line; do
        name=${line#*  }
        mkdir -p -- "$to/$(dirname -- "$name")"
        cp -p -- "$from/$name" "$to/$name"
    done < "$list"
}

if [ "$action" = remove ]; then
    [ -d "$state" ] || die 'No installer-managed plugin is installed.'
    targets "$state/plugin.sha256"
    verify "$root" "$state/plugin.sha256"
    # Changed user files are never removed silently. Core/model paths do not
    # occur in this package's ownership list.
    transaction=$(mktemp -d "$root/.llama-transaction.XXXXXX")
    mkdir "$transaction/files"
    cp "$state/plugin.sha256" "$transaction/old.sha256"
    copy_files "$root" "$transaction/old.sha256" "$transaction/files"
    # shellcheck disable=SC2329 # Invoked by the EXIT trap below.
    restore_remove() {
        code=$?
        trap - EXIT HUP INT TERM
        copy_files "$transaction/files" "$transaction/old.sha256" "$root"
        if [ -d "$transaction/state" ]; then mv "$transaction/state" "$state"; fi
        rm -rf -- "$transaction"
        rmdir "$lock"
        exit "$code"
    }
    trap restore_remove EXIT
    remove_files "$state/plugin.sha256"
    mv "$state" "$transaction/state"
    trap - EXIT HUP INT TERM
    rm -rf -- "$transaction"
    rmdir "$lock"
    echo "Removed llama.rexx; cREXX and models retained: $root"
    exit 0
fi
verify "$root" "$here/core.sha256"
verify "$here/payload" "$here/plugin.sha256"
targets "$here/plugin.sha256"
if [ -d "$state" ]; then
    [ "$(cat "$state/backend")" = "$(cat "$here/backend")" ] ||
        die 'Remove the existing backend before installing a different one.'
    targets "$state/plugin.sha256"
    verify "$root" "$state/plugin.sha256"
fi
while IFS= read -r line; do
    name=${line#*  }
    if [ -e "$root/$name" ]; then
        [ -d "$state" ] || die "Refusing to overwrite an unowned file: $name"
        if ! cut -c 67- "$state/plugin.sha256" | grep -Fqx -- "$name"; then
            die "Refusing to overwrite an unowned file: $name"
        fi
    fi
done < "$here/plugin.sha256"
echo "Verified matching cREXX installation: $root"
[ "$action" = install ] || exit 0

transaction=$(mktemp -d "$root/.llama-transaction.XXXXXX")
mkdir "$transaction/old-files" "$transaction/new-state"
if [ -d "$state" ]; then
    cp "$state/plugin.sha256" "$transaction/old.sha256"
    copy_files "$root" "$transaction/old.sha256" "$transaction/old-files"
fi
for name in manage.sh installer.json core.sha256 plugin.sha256 backend; do
    cp -p "$here/$name" "$transaction/new-state/$name"
done
rollback() {
    code=$?
    trap - EXIT HUP INT TERM
    remove_files "$here/plugin.sha256"
    if [ -f "$transaction/old.sha256" ]; then
        copy_files "$transaction/old-files" "$transaction/old.sha256" "$root"
    fi
    if [ -d "$transaction/old-state" ]; then
        rm -rf -- "$state"
        mv "$transaction/old-state" "$state"
    fi
    rm -rf -- "$transaction"
    rmdir "$lock"
    echo 'llama.rexx install failed; previous plugin files restored.' >&2
    exit "$code"
}
trap rollback EXIT
trap 'exit 1' HUP INT TERM
if [ -f "$transaction/old.sha256" ]; then remove_files "$transaction/old.sha256"; fi
copy_files "$here/payload" "$here/plugin.sha256" "$root"
verify "$root" "$here/plugin.sha256"
if [ -d "$state" ]; then mv "$state" "$transaction/old-state"; fi
mv "$transaction/new-state" "$state"
trap - EXIT HUP INT TERM
rm -rf -- "$transaction"
rmdir "$lock"
echo "Installed llama.rexx in $root"
