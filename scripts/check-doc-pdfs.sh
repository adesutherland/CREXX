#!/usr/bin/env bash
set -euo pipefail

asset_dir=${1:?Usage: check-doc-pdfs.sh ASSET_DIR TAG}
tag=${2:?}
for book in language-reference programming-guide vm-specification library-reference; do
  pdf="$asset_dir/CREXX-$tag-$book.pdf"
  if [ ! -s "$pdf" ]; then
    echo "Missing or empty book PDF: $pdf" >&2
    exit 1
  fi
done
