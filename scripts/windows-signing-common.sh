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
  local file_path count=0 selected_vm="" entry="$directory/bin/rxvm.exe"
  local scripts_dir
  scripts_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
  python3 "$scripts_dir/refresh-package-manifests.py" "$directory" --verify
  # rxvm is an exact copy of the selected VM on Windows. Signing the two
  # copies independently can give them different timestamps and break that
  # public-entry contract. Sign the selected binary once, then copy it.
  if [[ -f "$entry" ]]; then
    for file_path in "$directory/bin/rxbvm.exe" "$directory/bin/rxtvm.exe"; do
      if [[ -f "$file_path" ]] && cmp -s "$entry" "$file_path"; then
        selected_vm="$file_path"
        break
      fi
    done
  fi
  while IFS= read -r -d '' file_path; do
    if [[ -n "$selected_vm" && "$file_path" == "$entry" ]]; then continue; fi
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
  if [[ -n "$selected_vm" ]]; then cp "$selected_vm" "$entry"; fi
  [[ "$count" -gt 0 ]] || { echo "No signable Windows files in $directory" >&2; return 1; }
  echo "Verified signatures on $count Windows files."
  if [[ -d "$directory/bin/providers" ]]; then
    python3 "$scripts_dir/refresh-provider-manifests.py" "$directory/bin/providers"
  fi
  python3 "$scripts_dir/refresh-package-manifests.py" "$directory"
}
