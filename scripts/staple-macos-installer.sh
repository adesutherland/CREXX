#!/usr/bin/env bash
# Notarization acceptance can precede a temporary CloudKit ticket-service failure.
# Keep installer publication conditional on both stapling and ticket validation.
set -euo pipefail
if [[ $# -ne 1 ]]; then
  echo "Usage: $0 installer.pkg" >&2
  exit 64
fi

stapler_with_retry() {
  local action="$1" package="$2" attempt result delay
  for attempt in 1 2 3 4; do
    if xcrun stapler "$action" "$package"; then
      return 0
    else
      result=$?
    fi
    # EX_NOHOST (68) is stapler's ticket-service failure. Other errors require
    # repair, and exhausted retries must still fail the packaging job.
    if [[ $result -ne 68 || $attempt -eq 4 ]]; then
      return "$result"
    fi
    delay=$((15 * attempt))
    echo "Apple ticket service unavailable during ${action}; retrying in ${delay}s (attempt $((attempt + 1))/4)." >&2
    sleep "$delay"
  done
}

stapler_with_retry staple "$1"
stapler_with_retry validate "$1"
