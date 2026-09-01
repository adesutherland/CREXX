#!/usr/bin/env bash

set -euo pipefail

usage() {
  echo "usage: $0 --configuration Debug|Release --source-commit SHA --output DIR [--jobs N] [--profile NAME]" >&2
}

configuration=""
source_commit=""
output_dir=""
jobs=5
profile="public-purpose-lab"

while (($#)); do
  case "$1" in
    --configuration)
      configuration="${2:-}"
      shift 2
      ;;
    --source-commit)
      source_commit="${2:-}"
      shift 2
      ;;
    --output)
      output_dir="${2:-}"
      shift 2
      ;;
    --jobs)
      jobs="${2:-}"
      shift 2
      ;;
    --profile)
      profile="${2:-}"
      shift 2
      ;;
    *)
      usage
      exit 2
      ;;
  esac
done

if [[ "$configuration" != "Debug" && "$configuration" != "Release" ]]; then
  usage
  exit 2
fi
if [[ ! "$source_commit" =~ ^[0-9a-f]{40}$ ]]; then
  echo "error: --source-commit must be a full Git SHA" >&2
  exit 2
fi
if [[ -z "$output_dir" || ! "$jobs" =~ ^[1-9][0-9]*$ ]]; then
  usage
  exit 2
fi

repo_root="$(git rev-parse --show-toplevel)"
resolved_commit="$(git -C "$repo_root" rev-parse "$source_commit^{commit}")"
if [[ "$resolved_commit" != "$source_commit" ]]; then
  echo "error: requested source commit did not resolve exactly" >&2
  exit 2
fi

mkdir -p "$output_dir"
if find "$output_dir" -mindepth 1 -print -quit | grep -q .; then
  echo "error: output directory must be empty: $output_dir" >&2
  exit 2
fi
output_dir="$(cd "$output_dir" && pwd -P)"

if ! minikube status -p "$profile" >/dev/null 2>&1; then
  echo "error: Minikube profile is not running: $profile" >&2
  exit 1
fi

namespace="crexx-build-audit-$(printf '%s' "$configuration" | tr '[:upper:]' '[:lower:]')-$(date -u +%Y%m%d%H%M%S)-$$"
kube=(kubectl --context "$profile")
cleanup_namespace() {
  if [[ "${KEEP_CREXX_BUILD_AUDIT_NAMESPACE:-0}" == "1" ]]; then
    echo "Retaining namespace $namespace because KEEP_CREXX_BUILD_AUDIT_NAMESPACE=1"
    return
  fi
  "${kube[@]}" delete namespace "$namespace" --wait=false >/dev/null 2>&1 || true
}
trap cleanup_namespace EXIT

"${kube[@]}" create namespace "$namespace" >/dev/null
"${kube[@]}" apply -n "$namespace" -f "$repo_root/tools/build-system/minikube-builder-pod.yaml" >/dev/null
"${kube[@]}" wait -n "$namespace" --for=condition=Ready pod/crexx-build-audit --timeout=10m >/dev/null

ready=0
for _attempt in {1..120}; do
  if "${kube[@]}" exec -n "$namespace" crexx-build-audit -- test -f /work/toolchain-ready >/dev/null 2>&1; then
    ready=1
    break
  fi
  pod_phase="$("${kube[@]}" get -n "$namespace" pod crexx-build-audit -o jsonpath='{.status.phase}')"
  if [[ "$pod_phase" == "Failed" || "$pod_phase" == "Succeeded" ]]; then
    "${kube[@]}" logs -n "$namespace" crexx-build-audit >&2 || true
    echo "error: Minikube builder terminated during toolchain setup" >&2
    exit 1
  fi
  sleep 5
done
if [[ "$ready" != "1" ]]; then
  "${kube[@]}" logs -n "$namespace" crexx-build-audit >&2 || true
  echo "error: Minikube toolchain setup timed out" >&2
  exit 1
fi

COPYFILE_DISABLE=1 git -C "$repo_root" archive "$source_commit" |
  "${kube[@]}" exec -i -n "$namespace" crexx-build-audit -- \
    tar --warning=no-unknown-keyword -xf - -C /work/source
COPYFILE_DISABLE=1 tar -C "$repo_root/tools/build-system" --exclude='__pycache__' -cf - . |
  "${kube[@]}" exec -i -n "$namespace" crexx-build-audit -- \
    tar --warning=no-unknown-keyword -xf - -C /work/tools

"${kube[@]}" exec -n "$namespace" crexx-build-audit -- \
  python3 /work/tools/capture_build_graph.py \
    --source /work/source \
    --build /work/build \
    --evidence /work/evidence \
    --source-commit "$source_commit" \
    --configuration "$configuration" \
    --jobs "$jobs" \
    --allow-source-without-git

"${kube[@]}" exec -n "$namespace" crexx-build-audit -- tar -C /work/evidence -cf - . |
  COPYFILE_DISABLE=1 tar -C "$output_dir" -xf -

echo "$output_dir"
