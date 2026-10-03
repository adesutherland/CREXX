#!/usr/bin/env bash
# CI/local orchestration only; book conversion and highlighting are cREXX.
set -euo pipefail
repo=${1:?Usage: build-docs.sh REPO WORK VERSION ASSET_TAG COMMIT [CHANNEL]}
work=${2:?}
version=${3:?}
asset_tag=${4:?}
commit=${5:?}
channel=${6:-local}
repo=$(cd "$repo" && pwd)
if [ -e "$work" ]; then
  echo "Documentation work directory must be fresh: $work" >&2
  exit 2
fi
mkdir -p "$work/logs" "$work/port/imports"
work=$(cd "$work" && pwd)
cd "$repo"
test "$(git rev-parse HEAD)" = "$commit"

{
  uname -srm
  for tool in pandoc xelatex biber makeindex xdvipdfmx inkscape gs cmake; do
    "$tool" --version 2>&1 | head -n 5 || true
  done
  if command -v dpkg-query >/dev/null; then
    dpkg-query -W 'texlive*' biber inkscape ghostscript
  fi
  kpsewhich JuliaMono-Regular.ttf unifont-18.0.01.otf unifont_upper-18.0.01.otf
} > "$work/logs/dependency-versions.log" 2>&1

cmake -S "$repo" -B "$work/product" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_BUILD_CHANNEL="$channel" \
  -DCREXX_BUILD_COMMIT="$commit" -DCREXX_ALLOW_NETWORK_DOWNLOADS=ON \
  -DENABLE_LLAMA=OFF > "$work/logs/configure.log" 2>&1
cmake --build "$work/product" --target crexx rxc rxas rxlink rxvm rxvme rxdas rxdb rxcpack \
  --parallel 4 > "$work/logs/product-build.log" 2>&1
product="$work/product/bin"
export PATH="$product:$PATH"
port="$work/port"
{
  "$product/rxc" -i "$product" -o "$port/imports/bookhighlight" docs/texttools/bookhighlight.crexx
  "$product/rxas" -o "$port/imports/bookhighlight" "$port/imports/bookhighlight"
  "$product/rxc" -i "$port/imports;$product" -o "$port/imports/texttools" docs/texttools/texttools.crexx
  "$product/rxas" -o "$port/imports/texttools" "$port/imports/texttools"
  "$product/rxc" -i "$port/imports;$product" -o "$port/generate-books" docs/generate-books.crexx
  "$product/rxas" -o "$port/generate-books" "$port/generate-books"
  "$product/rxlink" -o "$port/generate-books-linked" \
    "$port/generate-books.rxbin" "$port/imports/texttools.rxbin" \
    "$port/imports/bookhighlight.rxbin" "$product/library.rxbin"
} > "$work/logs/port-build.log" 2>&1
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  build "$repo" "$work/books" all initial "$version" > "$work/logs/books.log" 2>&1
asset_dir="$work/release-assets"
mkdir -p "$asset_dir"
for entry in \
  crexx_language_reference:language-reference \
  crexx_programming_guide:programming-guide \
  crexx_vm_spec:vm-specification \
  crexx_library_reference:library-reference; do
  book=${entry%%:*}
  name=${entry#*:}
  cp "$work/books/docs/books/$book/tex/book/$book.pdf" \
    "$asset_dir/CREXX-$asset_tag-$name.pdf"
done
bash scripts/check-doc-pdfs.sh "$asset_dir" "$asset_tag"
