#!/usr/bin/env bash
set -euo pipefail

die() {
  echo "error: $*" >&2
  exit 1
}

usage() {
  cat <<'EOF'
Usage: scripts/sign-latest-windows-release.sh [options]

Downloads the latest GitHub release Windows ZIP, signs Windows binaries using
Certum SimplySign via Jsign, then uploads a sibling ZIP with "-signed" in the
asset name.

Options:
  -R, --repo OWNER/REPO       GitHub repository. Defaults to current repo.
  -t, --tag TAG              Release tag. Defaults to the latest release.
  -a, --asset NAME           Windows ZIP asset. Defaults to the only matching
                             unsigned Windows ZIP in the release.
      --delete-unsigned      Delete the original unsigned asset after the
                             signed asset is visible in the release.
      --dry-run              Show what would be done without signing/uploading.
      --keep-work            Keep the temporary working directory after exit.
      --work-dir DIR         Use DIR as the working directory instead of mktemp.
  -h, --help                 Show this help.

Environment:
  PROVIDER                   Path to SunPKCS11 provider config.
  CERTUM_ALIAS               Certificate alias/serial to use for signing.
  TSA_URL                    Timestamp authority URL.
EOF
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PROVIDER="${PROVIDER:-$SCRIPT_DIR/provider.macos.cfg}"
CERTUM_ALIAS="${CERTUM_ALIAS:-7DDC0FE9C4D43C9D1D900B39548410F1}"
TSA_URL="${TSA_URL:-http://time.certum.pl}"

REPO=""
TAG=""
ASSET=""
DELETE_UNSIGNED=0
DRY_RUN=0
KEEP_WORK=0
WORK=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    -R|--repo)
      [[ $# -ge 2 ]] || die "$1 requires OWNER/REPO"
      REPO="$2"
      shift 2
      ;;
    -t|--tag)
      [[ $# -ge 2 ]] || die "$1 requires TAG"
      TAG="$2"
      shift 2
      ;;
    -a|--asset)
      [[ $# -ge 2 ]] || die "$1 requires an asset name"
      ASSET="$2"
      shift 2
      ;;
    --delete-unsigned)
      DELETE_UNSIGNED=1
      shift
      ;;
    --dry-run)
      DRY_RUN=1
      shift
      ;;
    --keep-work)
      KEEP_WORK=1
      shift
      ;;
    --work-dir)
      [[ $# -ge 2 ]] || die "$1 requires DIR"
      WORK="$2"
      KEEP_WORK=1
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

for cmd in gh jsign osslsigncode unzip zip find file grep; do
  command -v "$cmd" >/dev/null || die "missing command: $cmd"
done

read_release_assets() {
  RELEASE_ASSETS=()
  local asset_name
  while IFS= read -r asset_name; do
    [[ -n "$asset_name" ]] && RELEASE_ASSETS+=("$asset_name")
  done < <(gh release view "$TAG" -R "$REPO" --json assets --jq '.assets[].name')
}

[[ -f "$PROVIDER" ]] || die "provider config not found: $PROVIDER"
gh auth status >/dev/null || die "GitHub CLI is not logged in; run: gh auth login -h github.com -p https -w"

if [[ -z "$REPO" ]]; then
  REPO="$(cd "$REPO_ROOT" && gh repo view --json nameWithOwner --jq .nameWithOwner 2>/dev/null)" ||
    die "could not infer repo; pass --repo OWNER/REPO"
fi

if [[ -z "$TAG" ]]; then
  TAG="$(gh release view -R "$REPO" --json tagName --jq .tagName)" ||
    die "could not determine latest release for $REPO"
fi

read_release_assets

asset_exists() {
  local name="$1"
  local existing
  for existing in "${RELEASE_ASSETS[@]}"; do
    [[ "$existing" == "$name" ]] && return 0
  done
  return 1
}

if [[ -z "$ASSET" ]]; then
  CANDIDATES=()
  while IFS= read -r candidate; do
    [[ -n "$candidate" ]] && CANDIDATES+=("$candidate")
  done < <(
      printf '%s\n' "${RELEASE_ASSETS[@]}" |
        grep -Ei '\.zip$' |
        grep -Evi -- '(^|[-_.])signed([-_.]|\.zip$)' |
        grep -Ei 'win|windows'
    )

  if [[ "${#CANDIDATES[@]}" -eq 0 ]]; then
    printf 'Release assets:\n' >&2
    printf '  %s\n' "${RELEASE_ASSETS[@]}" >&2
    die "no unsigned Windows ZIP asset found; pass --asset NAME"
  fi

  if [[ "${#CANDIDATES[@]}" -gt 1 ]]; then
    printf 'Candidate Windows ZIP assets:\n' >&2
    printf '  %s\n' "${CANDIDATES[@]}" >&2
    die "more than one Windows ZIP asset matched; pass --asset NAME"
  fi

  ASSET="${CANDIDATES[0]}"
fi

asset_exists "$ASSET" || die "asset not found in $REPO@$TAG: $ASSET"

is_windows_signable() {
  local file_path="$1"
  local lower_path
  local file_type

  file_type="$(file -b "$file_path")"
  if [[ "$file_type" == PE32* ]]; then
    return 0
  fi

  lower_path="$(printf '%s' "$file_path" | tr '[:upper:]' '[:lower:]')"
  case "$lower_path" in
    *.msi|*.cab|*.cat|*.appx|*.msix|*.ps1|*.ps1xml|*.psc1|*.psd1|*.psm1|*.cdxml|*.mof|*.js|*.vbs|*.wsf)
      return 0
      ;;
  esac

  return 1
}

if [[ "$ASSET" != *.* ]]; then
  SIGNED_ASSET="${ASSET}-signed"
else
  ASSET_BASE="${ASSET%.*}"
  ASSET_EXT="${ASSET##*.}"
  SIGNED_ASSET="${ASSET_BASE}-signed.${ASSET_EXT}"
fi

echo "Repository:       $REPO"
echo "Release tag:      $TAG"
echo "Unsigned asset:   $ASSET"
echo "Signed asset:     $SIGNED_ASSET"
echo "Provider config:  $PROVIDER"
echo "Certum alias:     $CERTUM_ALIAS"
echo "Timestamp URL:    $TSA_URL"

if [[ "$DRY_RUN" -eq 1 ]]; then
  echo "Dry run only; no files changed."
  exit 0
fi

if [[ -z "$WORK" ]]; then
  WORK="$(mktemp -d "${TMPDIR:-/tmp}/crexx-codesign.XXXXXX")"
else
  mkdir -p "$WORK"
  WORK="$(cd "$WORK" && pwd)"
fi

if [[ "$KEEP_WORK" -eq 1 ]]; then
  echo "Working directory: $WORK"
else
  trap 'rm -rf "$WORK"' EXIT
fi

DOWNLOAD_DIR="$WORK/download"
UNPACKED_DIR="$WORK/unpacked"
mkdir -p "$DOWNLOAD_DIR" "$UNPACKED_DIR"

echo "Downloading $ASSET"
gh release download "$TAG" -R "$REPO" -p "$ASSET" -D "$DOWNLOAD_DIR" --clobber

INPUT_ZIP="$DOWNLOAD_DIR/$ASSET"
[[ -f "$INPUT_ZIP" ]] || die "downloaded asset not found: $INPUT_ZIP"

echo "Unpacking"
unzip -q "$INPUT_ZIP" -d "$UNPACKED_DIR"

signed_count=0
skipped_count=0

while IFS= read -r -d '' file; do
  rel="${file#$UNPACKED_DIR/}"

  if ! is_windows_signable "$file"; then
    continue
  fi

  if osslsigncode verify -in "$file" >/dev/null 2>&1; then
    echo "Already signed: $rel"
    skipped_count=$((skipped_count + 1))
    continue
  fi

  echo "Signing: $rel"
  jsign --verbose \
    --storetype PKCS11 \
    --keystore "$PROVIDER" \
    --alias "$CERTUM_ALIAS" \
    --alg SHA-256 \
    --tsaurl "$TSA_URL" \
    "$file"

  osslsigncode verify -in "$file" >/dev/null
  signed_count=$((signed_count + 1))
done < <(find "$UNPACKED_DIR" -type f -print0)

[[ "$signed_count" -gt 0 || "$skipped_count" -gt 0 ]] ||
  die "no signable Windows binaries found in $ASSET"

OUTPUT_ZIP="$WORK/$SIGNED_ASSET"
echo "Creating $SIGNED_ASSET"
(
  cd "$UNPACKED_DIR"
  export COPYFILE_DISABLE=1
  zip -qr -X "$OUTPUT_ZIP" .
)

echo "Uploading $SIGNED_ASSET"
gh release upload "$TAG" "$OUTPUT_ZIP" -R "$REPO" --clobber

read_release_assets
asset_exists "$SIGNED_ASSET" || die "signed asset upload is not visible in the release: $SIGNED_ASSET"

if [[ "$DELETE_UNSIGNED" -eq 1 ]]; then
  [[ "$ASSET" != "$SIGNED_ASSET" ]] || die "refusing to delete asset because signed and unsigned names are identical"
  asset_exists "$ASSET" || die "unsigned asset is already absent: $ASSET"

  echo "Deleting unsigned asset $ASSET"
  gh release delete-asset "$TAG" "$ASSET" -R "$REPO" --yes

  read_release_assets
  asset_exists "$SIGNED_ASSET" || die "signed asset disappeared after deleting unsigned asset"
  if asset_exists "$ASSET"; then
    die "unsigned asset still exists after delete attempt: $ASSET"
  fi
fi

echo "Done. Signed $signed_count file(s), skipped $skipped_count already-signed file(s)."
