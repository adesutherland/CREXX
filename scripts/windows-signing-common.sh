#!/usr/bin/env bash

is_windows_signable() {
  local file_path="$1" file_type lower_path
  file_type="$(file -b "$file_path")"
  [[ "$file_type" != PE32* ]] || return 0
  lower_path="$(printf '%s' "$file_path" | tr '[:upper:]' '[:lower:]')"
  case "$lower_path" in
    *.msi|*.cab|*.cat|*.appx|*.msix|*.ps1|*.ps1xml|*.psc1|*.psd1|*.psm1|*.cdxml|*.mof|*.js|*.vbs|*.wsf) return 0 ;;
  esac
  return 1
}

sign_windows_payload() {
  local directory="$1" provider="$2" certum_alias="$3" tsa_url="$4"
  local file_path count=0
  while IFS= read -r -d '' file_path; do
    if ! is_windows_signable "$file_path"; then
      continue
    fi
    if ! osslsigncode verify -in "$file_path" >/dev/null 2>&1; then
      echo "Signing: ${file_path#"$directory"/}"
      jsign --storetype PKCS11 --keystore "$provider" --alias "$certum_alias" \
        --alg SHA-256 --tsaurl "$tsa_url" "$file_path"
    fi
    osslsigncode verify -in "$file_path" >/dev/null
    count=$((count + 1))
  done < <(find "$directory" -type f -print0)
  [[ "$count" -gt 0 ]] || { echo "No signable Windows files in $directory" >&2; return 1; }
  echo "Verified signatures on $count Windows files."
}
