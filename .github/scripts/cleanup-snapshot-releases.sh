#!/usr/bin/env bash

set -euo pipefail

AGE_LIMIT_DAYS="${AGE_LIMIT_DAYS:-30}"
RELEASE_NAME_TO_CLEAN="${RELEASE_NAME_TO_CLEAN:-Development Snapshot}"
DRY_RUN="${DRY_RUN:-0}"

if [[ -z "${GH_REPO:-}" ]]; then
  echo "GH_REPO must be set to an owner/repository value." >&2
  exit 1
fi

if ! [[ "$AGE_LIMIT_DAYS" =~ ^[0-9]+$ ]]; then
  echo "AGE_LIMIT_DAYS must be a non-negative integer. Got: '$AGE_LIMIT_DAYS'" >&2
  exit 1
fi

days_ago_to_epoch() {
  local days="$1"

  if date -u -d "-${days} days" +%s >/dev/null 2>&1; then
    date -u -d "-${days} days" +%s
  else
    date -u -v-"${days}"d +%s
  fi
}

epoch_to_iso() {
  local epoch="$1"

  if date -u -d "@${epoch}" +"%Y-%m-%dT%H:%M:%SZ" >/dev/null 2>&1; then
    date -u -d "@${epoch}" +"%Y-%m-%dT%H:%M:%SZ"
  else
    date -u -r "${epoch}" +"%Y-%m-%dT%H:%M:%SZ"
  fi
}

iso_to_epoch() {
  local iso_timestamp="$1"

  if date -u -d "$iso_timestamp" +%s >/dev/null 2>&1; then
    date -u -d "$iso_timestamp" +%s
  else
    date -ju -f "%Y-%m-%dT%H:%M:%SZ" "$iso_timestamp" +%s
  fi
}

dry_run_value=$(printf '%s' "$DRY_RUN" | tr '[:upper:]' '[:lower:]')

case "$dry_run_value" in
  1|true|yes)
    dry_run=1
    ;;
  *)
    dry_run=0
    ;;
esac

cutoff_seconds=$(days_ago_to_epoch "$AGE_LIMIT_DAYS")
cutoff_iso=$(epoch_to_iso "$cutoff_seconds")
matched_count=0
kept_count=0
deleted_count=0

echo "Checking for releases named '$RELEASE_NAME_TO_CLEAN' older than $AGE_LIMIT_DAYS days in '$GH_REPO'..."
echo "Cutoff date is $cutoff_iso."

while IFS= read -r release_json; do
  matched_count=$((matched_count + 1))

  tag=$(jq -r '.tag_name' <<<"$release_json")
  release_time=$(jq -r '.published_at // .created_at' <<<"$release_json")
  release_seconds=$(iso_to_epoch "$release_time")

  if (( release_seconds < cutoff_seconds )); then
    if (( dry_run )); then
      echo "Would delete release with tag '$tag' (published on $release_time)..."
    else
      echo "Deleting release with tag '$tag' (published on $release_time)..."
      gh release delete "$tag" --repo "$GH_REPO" --yes --cleanup-tag
    fi
    deleted_count=$((deleted_count + 1))
  else
    echo "Keeping release with tag '$tag' (published on $release_time)..."
    kept_count=$((kept_count + 1))
  fi
done < <(
  gh api --paginate "repos/${GH_REPO}/releases?per_page=100" |
    jq -c --arg release_name "$RELEASE_NAME_TO_CLEAN" '.[] | select(.name == $release_name)'
)

if (( matched_count == 0 )); then
  echo "No releases found matching '$RELEASE_NAME_TO_CLEAN'."
fi

if (( dry_run )); then
  echo "Dry run complete. Matched: $matched_count, would delete: $deleted_count, would keep: $kept_count."
else
  echo "Cleanup complete. Matched: $matched_count, deleted: $deleted_count, kept: $kept_count."
fi
