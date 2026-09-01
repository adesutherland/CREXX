#!/usr/bin/env bash

run_windows_installer_packager() {
  local usage_name="$1"
  local default_tag_mode="$2"
  local default_tag_help="$3"
  local default_sign_mode="$4"
  local description="$5"
  shift 5

  die() {
    echo "error: $*" >&2
    exit 1
  }

  require_cmd() {
    command -v "$1" >/dev/null || die "missing command: $1"
  }

  trim_cr() {
    printf '%s' "$1" | tr -d '\r'
  }

  display_to_label() {
    local value="$1"
    value="$(trim_cr "$value")"
    value="${value#crexx-}"
    value="${value#CREXX-}"
    value="${value#v}"
    printf '%s' "$value"
  }

  derive_file_version() {
    local value core major minor patch build part
    value="$(display_to_label "$1")"
    core="${value%%[-+]*}"
    IFS=. read -r major minor patch build <<EOF
$core
EOF
    major="${major:-0}"
    minor="${minor:-0}"
    patch="${patch:-0}"
    build="${build:-0}"

    for part in "$major" "$minor" "$patch" "$build"; do
      [[ "$part" =~ ^[0-9]+$ ]] || {
        printf '0.0.0.0'
        return
      }
    done

    printf '%s.%s.%s.%s' "$major" "$minor" "$patch" "$build"
  }

  usage() {
    local sign_default
    if [[ "$default_sign_mode" == "sign" ]]; then
      sign_default="Default: sign."
    else
      sign_default="Default: unsigned."
    fi

    cat <<EOF
Usage: $usage_name [options]

$description

Input options:
  --zip PATH                 Use an already-downloaded Windows ZIP.
  -R, --repo OWNER/REPO      GitHub repository. Defaults to the current repo.
  -t, --tag TAG              Release tag. $default_tag_help
  -a, --asset NAME           Release asset to download. Defaults to the only
                             Windows ZIP, preferring a signed ZIP when present.

Output options:
  -o, --output PATH          Installer path. Defaults in the current directory
                             as <zip-base>-setup.exe.
      --output-dir DIR       Directory for the default installer filename.
      --upload               Upload the generated installer to the release.
      --no-upload            Do not upload. This is the default.

Signing options:
      --sign                 Sign the embedded uninstaller and final setup.exe.
      --unsigned             Build without signing. $sign_default
      --provider PATH        SunPKCS11 provider config.
      --certum-alias TEXT    Certificate alias/serial.
      --tsa-url URL          Timestamp authority URL.

Installer UI options:
      --wizard-bitmap PATH   BMP used on welcome/finish pages.
      --no-wizard-bitmap     Use the default NSIS welcome/finish image.

Debug options:
      --keep-work            Keep the temporary work directory.
      --work-dir DIR         Use DIR as the work directory and keep it.
  -h, --help                 Show this help.
EOF
  }

  local script_dir repo_root
  script_dir="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)"
  repo_root="$(cd "$script_dir/.." && pwd)"

  local provider certum_alias tsa_url
  provider="${PROVIDER:-$script_dir/provider.macos.cfg}"
  certum_alias="${CERTUM_ALIAS:-7DDC0FE9C4D43C9D1D900B39548410F1}"
  tsa_url="${TSA_URL:-http://time.certum.pl}"

  local repo tag asset zip_path output output_dir display_version file_version
  local work keep_work sign_installer upload wizard_bitmap use_wizard_bitmap
  repo=""
  tag=""
  asset=""
  zip_path=""
  output=""
  output_dir=""
  display_version=""
  file_version=""
  work=""
  keep_work=0
  upload=0
  use_wizard_bitmap=1
  wizard_bitmap="$repo_root/packaging/windows/assets/crexx-wizard.bmp"

  if [[ "$default_sign_mode" == "sign" ]]; then
    sign_installer=1
  else
    sign_installer=0
  fi

  while [[ $# -gt 0 ]]; do
    case "$1" in
      --zip)
        [[ $# -ge 2 ]] || die "$1 requires PATH"
        zip_path="$2"
        shift 2
        ;;
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
        [[ $# -ge 2 ]] || die "$1 requires NAME"
        asset="$2"
        shift 2
        ;;
      -o|--output)
        [[ $# -ge 2 ]] || die "$1 requires PATH"
        output="$2"
        shift 2
        ;;
      --output-dir)
        [[ $# -ge 2 ]] || die "$1 requires DIR"
        output_dir="$2"
        shift 2
        ;;
      --display-version)
        [[ $# -ge 2 ]] || die "$1 requires TEXT"
        display_version="$2"
        shift 2
        ;;
      --file-version)
        [[ $# -ge 2 ]] || die "$1 requires A.B.C.D"
        file_version="$2"
        shift 2
        ;;
      --upload)
        upload=1
        shift
        ;;
      --no-upload)
        upload=0
        shift
        ;;
      --sign)
        sign_installer=1
        shift
        ;;
      --unsigned)
        sign_installer=0
        shift
        ;;
      --provider)
        [[ $# -ge 2 ]] || die "$1 requires PATH"
        provider="$2"
        shift 2
        ;;
      --certum-alias)
        [[ $# -ge 2 ]] || die "$1 requires TEXT"
        certum_alias="$2"
        shift 2
        ;;
      --tsa-url)
        [[ $# -ge 2 ]] || die "$1 requires URL"
        tsa_url="$2"
        shift 2
        ;;
      --wizard-bitmap)
        [[ $# -ge 2 ]] || die "$1 requires PATH"
        wizard_bitmap="$2"
        use_wizard_bitmap=1
        shift 2
        ;;
      --no-wizard-bitmap)
        use_wizard_bitmap=0
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

  require_cmd makensis
  require_cmd unzip
  require_cmd find
  require_cmd du
  require_cmd awk

  local nsis_script
  nsis_script="$repo_root/packaging/windows/crexx.nsi"
  [[ -f "$nsis_script" ]] || die "NSIS script not found: $nsis_script"

  if [[ -n "$zip_path" && ( -n "$tag" || -n "$asset" ) ]]; then
    die "--zip cannot be combined with --tag or --asset"
  fi

  if [[ -n "$output" && -n "$output_dir" ]]; then
    die "--output cannot be combined with --output-dir"
  fi

  if [[ "$upload" -eq 1 && -n "$zip_path" ]]; then
    die "--upload requires a GitHub release input, not --zip"
  fi

  if [[ "$sign_installer" -eq 1 ]]; then
    require_cmd jsign
    require_cmd osslsigncode
    [[ -f "$provider" ]] || die "provider config not found: $provider"
  fi

  if [[ "$use_wizard_bitmap" -eq 1 ]]; then
    [[ -f "$wizard_bitmap" ]] || die "wizard bitmap not found: $wizard_bitmap"
    wizard_bitmap="$(cd "$(dirname "$wizard_bitmap")" && pwd)/$(basename "$wizard_bitmap")"
  fi

  if [[ -z "$work" ]]; then
    work="$(mktemp -d "${TMPDIR:-/tmp}/crexx-nsis.XXXXXX")"
  else
    mkdir -p "$work"
    work="$(cd "$work" && pwd)"
  fi

  if [[ "$keep_work" -eq 1 ]]; then
    echo "Working directory: $work"
  else
    trap "rm -rf -- $(printf '%q' "$work")" EXIT
  fi

  local download_dir unpack_dir
  download_dir="$work/download"
  unpack_dir="$work/unpacked"
  mkdir -p "$download_dir" "$unpack_dir"

  if [[ -z "$zip_path" ]]; then
    require_cmd gh

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

    local assets candidate lower
    if [[ -z "$asset" ]]; then
      assets="$(gh release view "$tag" -R "$repo" --json assets --jq '.assets[].name')" ||
        die "could not read release assets for $repo@$tag"

      local candidates signed_candidates
      candidates=()
      signed_candidates=()
      while IFS= read -r candidate; do
        [[ -n "$candidate" ]] || continue
        lower="$(printf '%s' "$candidate" | tr '[:upper:]' '[:lower:]')"
        case "$lower" in
          *.zip)
            if [[ "$lower" == *win* || "$lower" == *windows* ]]; then
              candidates+=("$candidate")
              if [[ "$lower" == *signed* ]]; then
                signed_candidates+=("$candidate")
              fi
            fi
            ;;
        esac
      done <<< "$assets"

      if [[ "${#signed_candidates[@]}" -eq 1 ]]; then
        asset="${signed_candidates[0]}"
      elif [[ "${#candidates[@]}" -eq 1 ]]; then
        asset="${candidates[0]}"
      else
        printf 'Candidate Windows ZIP assets for %s@%s:\n' "$repo" "$tag" >&2
        if [[ "${#candidates[@]}" -gt 0 ]]; then
          printf '  %s\n' "${candidates[@]}" >&2
        else
          printf '  (none)\n' >&2
        fi
        die "pass --asset NAME"
      fi
    fi

    echo "Downloading $repo@$tag asset $asset"
    gh release download "$tag" -R "$repo" -p "$asset" -D "$download_dir" --clobber
    zip_path="$download_dir/$asset"
  fi

  [[ -f "$zip_path" ]] || die "ZIP not found: $zip_path"

  echo "Unpacking $(basename "$zip_path")"
  unzip -q "$zip_path" -d "$unpack_dir"

  local payload_dir top_dir_count dir
  payload_dir=""
  top_dir_count=0
  while IFS= read -r dir; do
    top_dir_count=$((top_dir_count + 1))
    payload_dir="$dir"
  done < <(find "$unpack_dir" -mindepth 1 -maxdepth 1 -type d)

  if [[ "$top_dir_count" -eq 1 && -d "$payload_dir/bin" ]]; then
    :
  elif [[ -d "$unpack_dir/bin" ]]; then
    payload_dir="$unpack_dir"
  else
    find "$unpack_dir" -maxdepth 2 -type d -print >&2
    die "could not find a CREXX payload directory with a bin subdirectory"
  fi

  local exe
  for exe in crexx.exe rxc.exe rxas.exe rxlink.exe rxvm.exe; do
    [[ -f "$payload_dir/bin/$exe" ]] || die "payload is missing bin/$exe"
  done

  if [[ -z "$display_version" ]]; then
    if [[ -f "$payload_dir/VERSION" ]]; then
      display_version="$(sed -n '1p' "$payload_dir/VERSION")"
    elif [[ -n "$tag" ]]; then
      display_version="$tag"
    else
      display_version="dev"
    fi
  fi
  display_version="$(display_to_label "$display_version")"

  if [[ -z "$file_version" ]]; then
    file_version="$(derive_file_version "$display_version")"
  fi

  if [[ ! "$file_version" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    die "--file-version must be numeric A.B.C.D, got: $file_version"
  fi

  local zip_base
  if [[ -z "$output" ]]; then
    zip_base="$(basename "$zip_path")"
    zip_base="${zip_base%.zip}"
    zip_base="${zip_base%-signed}"
    if [[ -n "$output_dir" ]]; then
      output="$output_dir/${zip_base}-setup.exe"
    else
      output="$PWD/${zip_base}-setup.exe"
    fi
  fi

  local abs_output_dir
  abs_output_dir="$(dirname "$output")"
  mkdir -p "$abs_output_dir"
  output="$(cd "$abs_output_dir" && pwd)/$(basename "$output")"

  local estimated_size_kb
  estimated_size_kb="$(du -sk "$payload_dir" | awk '{print $1}')"

  local sign_helper
  sign_helper=""
  if [[ "$sign_installer" -eq 1 ]]; then
    sign_helper="$work/sign-windows-file.sh"
    {
      echo '#!/usr/bin/env bash'
      echo 'set -euo pipefail'
      printf 'provider=%q\n' "$provider"
      printf 'certum_alias=%q\n' "$certum_alias"
      printf 'tsa_url=%q\n' "$tsa_url"
      echo 'target="${1:?target file is required}"'
      echo 'jsign --verbose --storetype PKCS11 --keystore "$provider" --alias "$certum_alias" --alg SHA-256 --tsaurl "$tsa_url" "$target"'
      echo 'osslsigncode verify -in "$target" >/dev/null'
    } > "$sign_helper"
    chmod +x "$sign_helper"
  fi

  echo "Payload:         $payload_dir"
  echo "Version label:   $display_version"
  echo "File version:    $file_version"
  echo "Estimated size:  ${estimated_size_kb} KB"
  echo "Output:          $output"
  if [[ "$use_wizard_bitmap" -eq 1 ]]; then
    echo "Wizard bitmap:   $wizard_bitmap"
  else
    echo "Wizard bitmap:   NSIS default"
  fi
  if [[ "$sign_installer" -eq 1 ]]; then
    echo "Signing:         enabled"
  else
    echo "Signing:         disabled"
  fi

  local makensis_args
  makensis_args=(
    "-DCREXX_PAYLOAD_DIR=$payload_dir"
    "-DCREXX_OUTFILE=$output"
    "-DCREXX_DISPLAY_VERSION=$display_version"
    "-DCREXX_FILE_VERSION=$file_version"
    "-DCREXX_ESTIMATED_SIZE_KB=$estimated_size_kb"
  )

  if [[ "$use_wizard_bitmap" -eq 1 ]]; then
    makensis_args+=("-DCREXX_WIZARD_BITMAP=$wizard_bitmap")
  fi

  if [[ "$sign_installer" -eq 1 ]]; then
    makensis_args+=("-DCREXX_SIGN_HELPER=$sign_helper")
  fi

  makensis "${makensis_args[@]}" "$nsis_script"

  [[ -f "$output" ]] || die "installer was not created: $output"

  if [[ "$sign_installer" -eq 1 ]]; then
    echo "Signing installer: $output"
    "$sign_helper" "$output"
    osslsigncode verify -in "$output" >/dev/null
  fi

  if [[ "$upload" -eq 1 ]]; then
    require_cmd gh
    [[ -n "$repo" && -n "$tag" ]] || die "internal error: upload requires repo and tag"
    echo "Uploading $(basename "$output") to $repo@$tag"
    gh release upload "$tag" "$output" -R "$repo" --clobber
  fi

  echo "Created $output"
}
