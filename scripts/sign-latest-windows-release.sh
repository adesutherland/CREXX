#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/sign-windows-asset-common.sh
source "$SCRIPT_DIR/sign-windows-asset-common.sh"

run_windows_release_asset_signer \
  "scripts/sign-latest-windows-release.sh" \
  "latest-release" \
  "Defaults to the latest release." \
  "Downloads the latest GitHub release Windows ZIP, signs Windows binaries using Certum SimplySign via Jsign, then uploads a sibling ZIP with \"-signed\" in the asset name." \
  "$@"
