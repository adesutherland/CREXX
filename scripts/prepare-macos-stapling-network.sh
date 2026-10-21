#!/usr/bin/env bash
# GitHub's Intel runner workaround for actions/runner-images#14924.
# HTTP/3 ticket requests lose their connection; force the HTTPS TCP fallback.
set -euo pipefail

if [[ "${GITHUB_ACTIONS:-}" != true ||
      "${RUNNER_ENVIRONMENT:-}" != github-hosted ||
      "$(uname -s)" != Darwin || "$(uname -m)" != x86_64 ]]; then
  echo 'Stapling network preparation requires a disposable GitHub-hosted Intel Mac.' >&2
  exit 64
fi

# This changes only the disposable runner, never a developer or self-hosted Mac.
# Keep ordinary HTTPS/TCP available and retain all notarization/stapling gates.
printf '%s\n' 'block return out quick proto udp from any to any port 443' |
  sudo pfctl -f -
sudo pfctl -E
