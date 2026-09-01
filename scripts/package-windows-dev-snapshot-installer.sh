#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/package-windows-installer-common.sh
source "$SCRIPT_DIR/package-windows-installer-common.sh"

run_windows_installer_packager \
  "scripts/package-windows-dev-snapshot-installer.sh" \
  "dev-snapshot" \
  "Defaults to the dev-snapshot release." \
  "sign" \
  "Download the CREXX Dev Snapshot Windows ZIP, build a CREXX NSIS setup.exe, sign the embedded uninstaller and final installer, and optionally upload it." \
  "$@"
