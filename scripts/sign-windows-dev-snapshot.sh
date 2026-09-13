#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/sign-windows-asset-common.sh
source "$SCRIPT_DIR/sign-windows-asset-common.sh"

run_windows_release_asset_signer \
  "scripts/sign-windows-dev-snapshot.sh" \
  "dev-snapshot" \
  "Defaults to the dev-snapshot release." \
  "Download the current Windows snapshot, sign its payload, embedded uninstaller and setup, and publish a signed ZIP and signed setup alongside the unsigned downloads." \
  "$@"
