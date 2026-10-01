#!/usr/bin/env bash

run_windows_release_asset_signer() {
  local usage_name="$1"
  local default_tag_mode="$2"
  local default_tag_help="$3"
  local description="$4"
  shift 4

  die() {
    echo "error: $*" >&2
    exit 1
  }

  usage() {
    cat <<EOF
Usage: $usage_name [options]

$description

Options:
  -R, --repo OWNER/REPO       GitHub repository. Defaults to current repo.
  -t, --tag TAG              Release tag. $default_tag_help
  -a, --asset NAME           Core or plugin Windows ZIP. Defaults to the core
                             unsigned Windows ZIP; select a plugin explicitly.
      --keep-unsigned        Keep the original unsigned asset after the signed
                             asset is visible in the release.
      --delete-unsigned      Delete the original ZIP after publication (versioned
                             releases only). Snapshots always retain both forms.
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

  local script_dir repo_root
  script_dir="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)"
  repo_root="$(cd "$script_dir/.." && pwd)"

  local provider certum_alias tsa_url
  provider="${PROVIDER:-$script_dir/provider.macos.cfg}"
  certum_alias="${CERTUM_ALIAS:-7DDC0FE9C4D43C9D1D900B39548410F1}"
  tsa_url="${TSA_URL:-http://time.certum.pl}"

  local repo tag asset delete_unsigned dry_run keep_work work
  repo=""
  tag=""
  asset=""
  delete_unsigned=1
  local with_installer=0 explicit_delete=0
  if [[ "$default_tag_mode" == "dev-snapshot" ]]; then
    delete_unsigned=0
    with_installer=1
  fi
  dry_run=0
  keep_work=0
  work=""

  while [[ $# -gt 0 ]]; do
    case "$1" in
      -R|--repo)
        [[ $# -ge 2 ]] || die "$1 requires OWNER/REPO"
        repo="$2"
        shift 2
        ;;
      -t|--tag)
        [[ $# -ge 2 ]] || die "$1 requires TAG"
        tag="$2"
        shift 2
        ;;
      -a|--asset)
        [[ $# -ge 2 ]] || die "$1 requires an asset name"
        asset="$2"
        shift 2
        ;;
      --delete-unsigned)
        explicit_delete=1
        delete_unsigned=1
        shift
        ;;
      --keep-unsigned)
        delete_unsigned=0
        shift
        ;;
      --dry-run)
        dry_run=1
        shift
        ;;
      --keep-work)
        keep_work=1
        shift
        ;;
      --work-dir)
        [[ $# -ge 2 ]] || die "$1 requires DIR"
        work="$2"
        keep_work=1
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

  # shellcheck source=scripts/windows-signing-common.sh
  source "$script_dir/windows-signing-common.sh"
  local release_helper="$script_dir/windows-release-assets.py"

  local cmd
  for cmd in gh jsign osslsigncode unzip zip find file grep python3; do
    command -v "$cmd" >/dev/null || die "missing command: $cmd"
  done

  [[ -f "$provider" ]] || die "provider config not found: $provider"
  gh auth status >/dev/null || die "GitHub CLI is not logged in; run: gh auth login -h github.com -p https -w"

  if [[ -z "$repo" ]]; then
    repo="$(cd "$repo_root" && gh repo view --json nameWithOwner --jq .nameWithOwner 2>/dev/null)" ||
      die "could not infer repo; pass --repo OWNER/REPO"
  fi

  if [[ -z "$tag" ]]; then
    if [[ "$default_tag_mode" == "latest-release" ]]; then
      tag="$(gh release view -R "$repo" --json tagName --jq .tagName)" ||
        die "could not determine latest release for $repo"
    else
      tag="$default_tag_mode"
    fi
  fi

  if [[ "$tag" == "dev-snapshot" ]]; then
    [[ "$explicit_delete" -eq 0 ]] || die "dev-snapshot retains unsigned assets; omit --delete-unsigned"
    delete_unsigned=0
    with_installer=1
  fi
  if [[ "$with_installer" -eq 1 ]]; then
    command -v makensis >/dev/null || die "missing command: makensis"
  fi

  local release_assets
  release_assets=()

  read_release_assets() {
    release_assets=()
    local asset_names
    local asset_name

    if ! asset_names="$(gh release view "$tag" -R "$repo" --json assets --jq '.assets[].name')"; then
      die "release not found or assets cannot be read: $repo@$tag"
    fi

    if [[ -n "$asset_names" ]]; then
      while IFS= read -r asset_name; do
        [[ -n "$asset_name" ]] && release_assets+=("$asset_name")
      done <<< "$asset_names"
    fi
  }

  read_release_assets

  asset_exists() {
    local name="$1"
    local existing
    [[ "${#release_assets[@]}" -gt 0 ]] || return 1

    for existing in "${release_assets[@]}"; do
      [[ "$existing" == "$name" ]] && return 0
    done
    return 1
  }

  if [[ -z "$asset" ]]; then
    local candidates
    candidates=()
    local candidate

    if [[ "${#release_assets[@]}" -gt 0 ]]; then
      while IFS= read -r candidate; do
        [[ -n "$candidate" ]] && candidates+=("$candidate")
      done < <(
        printf '%s\n' "${release_assets[@]}" |
          grep -Ei '\.zip$' |
          grep -Evi -- '(^|[-_.])signed([-_.]|\.zip$)' |
          grep -Ei '^CREXX-.*(win|windows)'
      )
    fi

    if [[ "${#candidates[@]}" -eq 0 ]]; then
      printf 'Release assets:\n' >&2
      if [[ "${#release_assets[@]}" -gt 0 ]]; then
        printf '  %s\n' "${release_assets[@]}" >&2
      else
        printf '  (none)\n' >&2
      fi
      die "no unsigned Windows ZIP asset found; pass --asset NAME"
    fi

    if [[ "${#candidates[@]}" -gt 1 ]]; then
      printf 'Candidate Windows ZIP assets:\n' >&2
      printf '  %s\n' "${candidates[@]}" >&2
      die "more than one Windows ZIP asset matched; pass --asset NAME"
    fi

    asset="${candidates[0]}"
  fi

  asset_exists "$asset" || die "asset not found in $repo@$tag: $asset"

  local signed_asset
  if [[ "$asset" != *.* ]]; then
    signed_asset="${asset}-signed"
  else
    local asset_base asset_ext
    asset_base="${asset%.*}"
    asset_ext="${asset##*.}"
    signed_asset="${asset_base}-signed.${asset_ext}"
  fi

  echo "Repository:       $repo"
  echo "Release tag:      $tag"
  echo "Unsigned asset:   $asset"
  echo "Signed asset:     $signed_asset"
  if [[ "$delete_unsigned" -eq 1 ]]; then
    echo "Unsigned cleanup: delete after signed upload is visible"
  else
    echo "Unsigned cleanup: keep original unsigned asset"
  fi
  echo "Provider config:  $provider"
  echo "Certum alias:     $certum_alias"
  echo "Timestamp URL:    $tsa_url"
  echo "Signed installer: $with_installer (snapshot signs payload, uninstaller and setup)"

  if [[ "$dry_run" -eq 1 ]]; then
    echo "Dry run only; no files changed."
    exit 0
  fi

  if [[ -z "$work" ]]; then
    work="$(mktemp -d "${TMPDIR:-/tmp}/crexx-codesign.XXXXXX")"
  else
    mkdir -p "$work"
    work="$(cd "$work" && pwd)"
  fi

  if [[ "$keep_work" -eq 1 ]]; then
    echo "Working directory: $work"
  else
    # Freeze the function-local path now; it is out of scope when EXIT runs.
    # shellcheck disable=SC2064
    trap "rm -rf -- $(printf '%q' "$work")" EXIT
  fi

  local download_dir unpacked_dir
  download_dir="$work/download"
  unpacked_dir="$work/unpacked"
  mkdir -p "$download_dir" "$unpacked_dir"

  local source_state="$work/source.json"
  python3 "$release_helper" capture --repo "$repo" --tag "$tag" --asset "$asset" --state "$source_state"
  local input_zip="$download_dir/$asset"
  python3 "$release_helper" download --state "$source_state" "$input_zip"
  [[ -f "$input_zip" ]] || die "downloaded asset not found: $input_zip"

  echo "Unpacking"
  unzip -q "$input_zip" -d "$unpacked_dir"

  local buildinfo payload_dir
  buildinfo="$(find "$unpacked_dir" -maxdepth 2 \( -name BUILDINFO -o -name llama-package.json \) -type f)"
  [[ -n "$buildinfo" && "$buildinfo" != *$'\n'* ]] || die "expected one core BUILDINFO or plugin manifest"
  payload_dir="$(dirname "$buildinfo")"
  if [[ -f "$payload_dir/llama-package.json" ]]; then
    with_installer=0
    echo 'Signing the plugin ZIP. Use sign-windows-packages.py for matching core/plugin installers.'
  fi
  python3 "$release_helper" payload --state "$source_state" "$payload_dir"
  sign_windows_payload "$payload_dir" "$provider" "$certum_alias" "$tsa_url"

  local output_zip
  output_zip="$work/$signed_asset"
  echo "Creating $signed_asset"
  (
    cd "$unpacked_dir" || exit
    export COPYFILE_DISABLE=1
    zip -qr -X "$output_zip" .
  )

  local publish_paths=("$output_zip")
  if [[ "$with_installer" -eq 1 ]]; then
    local output_installer="$work/${signed_asset%.zip}-setup.exe"
    bash "$script_dir/package-windows-nsis.sh" --zip "$output_zip" --sign --no-upload \
      --provider "$provider" --certum-alias "$certum_alias" --tsa-url "$tsa_url" \
      --output "$output_installer"
    publish_paths+=("$output_installer")
  fi

  echo "Publishing signed assets for the pinned source"
  python3 "$release_helper" publish --state "$source_state" "${publish_paths[@]}"

  read_release_assets
  asset_exists "$signed_asset" || die "signed asset upload is not visible in the release: $signed_asset"

  if [[ "$delete_unsigned" -eq 1 ]]; then
    [[ "$asset" != "$signed_asset" ]] || die "refusing to delete asset because signed and unsigned names are identical"
    asset_exists "$asset" || die "unsigned asset is already absent: $asset"

    echo "Deleting unsigned asset $asset"
    python3 "$release_helper" verify --state "$source_state"
    local source_id
    source_id="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["asset"]["id"])' "$source_state")"
    gh api "repos/$repo/releases/assets/$source_id" --method DELETE

    read_release_assets
    asset_exists "$signed_asset" || die "signed asset disappeared after deleting unsigned asset"
    if asset_exists "$asset"; then
      die "unsigned asset still exists after delete attempt: $asset"
    fi
  else
    echo "Keeping unsigned asset $asset"
  fi

  echo "Done. Signed assets are published for the pinned source commit."
}
