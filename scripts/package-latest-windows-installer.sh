#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/package-windows-installer-common.sh
source "$SCRIPT_DIR/package-windows-installer-common.sh"

run_windows_installer_packager \
  "scripts/package-latest-windows-installer.sh" \
  "latest-release" \
  "Defaults to the latest release." \
  "sign" \
  "Download the latest release Windows ZIP, build a CREXX NSIS setup.exe, sign the embedded uninstaller and final installer, and optionally upload it." \
  "$@"
