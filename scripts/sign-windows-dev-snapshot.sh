#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/sign-windows-asset-common.sh
source "$SCRIPT_DIR/sign-windows-asset-common.sh"

run_windows_release_asset_signer \
  "scripts/sign-windows-dev-snapshot.sh" \
  "dev-snapshot" \
  "Defaults to the dev-snapshot release." \
  "Downloads the CREXX Dev Snapshot Windows ZIP, signs Windows binaries using Certum SimplySign via Jsign, then uploads a sibling ZIP with \"-signed\" in the asset name." \
  "$@"
