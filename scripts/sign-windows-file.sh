#!/usr/bin/env bash
set -euo pipefail
scripts_dir="$(cd "$(dirname "$0")" && pwd)"
provider="${PROVIDER:-$scripts_dir/provider.macos.cfg}"
certum_alias="${CERTUM_ALIAS:-7DDC0FE9C4D43C9D1D900B39548410F1}"
tsa_url="${TSA_URL:-http://time.certum.pl}"
sign_file() {
  if ! osslsigncode verify -in "$1" >/dev/null 2>&1; then
    jsign --storetype PKCS11 --keystore "$provider" --alias "$certum_alias" \
      --alg SHA-256 --tsaurl "$tsa_url" "$1"
  fi
  osslsigncode verify -in "$1" >/dev/null
}
if [[ "${1:-}" == --nsis-plugins ]]; then
  mkdir -p "$3"
  for plugin in System.dll nsDialogs.dll nsExec.dll; do
    cp "$2/$plugin" "$3/$plugin"
    sign_file "$3/$plugin"
  done
else
  sign_file "${1:?target file is required}"
fi
