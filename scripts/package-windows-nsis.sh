#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/package-windows-installer-common.sh
source "$SCRIPT_DIR/package-windows-installer-common.sh"

run_windows_installer_packager \
  "scripts/package-windows-nsis.sh" \
  "latest-release" \
  "Defaults to the latest release." \
  "unsigned" \
  "Build a CREXX Windows x64 NSIS setup.exe from a release ZIP payload." \
  "$@"
