#!/usr/bin/env bash
# Ubuntu 24.04 dependencies for the initial free-font book profile.
set -euo pipefail
tools=${1:?Usage: install-doc-tools-linux.sh TOOL_DIRECTORY}
mkdir -p "$tools/downloads" "$tools/bin" "$tools/texmf/fonts/truetype/juliamono" \
  "$tools/texmf/fonts/opentype/unifont"

# The hosted Azure mirror took 84 minutes for the first 535 MB bootstrap.
# Keep local developer mirror choices; use Canonical's HTTPS archive on CI.
if [ "${GITHUB_ACTIONS:-}" = true ]; then
  for mirrors in /etc/apt/apt-mirrors.txt /etc/apt/apt-security-mirrors.txt; do
    if [ -f "$mirrors" ]; then
      sudo sed -i 's|http://azure\.archive\.ubuntu\.com/ubuntu|https://archive.ubuntu.com/ubuntu|g' "$mirrors"
    fi
  done
fi
apt_network=(-o Acquire::Retries=3 -o Acquire::http::Timeout=60 -o Acquire::https::Timeout=60)
sudo apt-get "${apt_network[@]}" update
sudo env DEBIAN_FRONTEND=noninteractive apt-get "${apt_network[@]}" install -y --no-install-recommends \
  build-essential cmake ninja-build libssl-dev pkg-config curl ca-certificates \
  texlive-xetex texlive-latex-extra texlive-fonts-recommended texlive-pstricks \
  texlive-bibtex-extra texlive-font-utils texlive-lang-english texlive-science biber \
  fonts-texgyre inkscape ghostscript poppler-utils
sudo mktexlsr
kpsewhich siunitx.sty || { echo "Missing required TeX package: siunitx.sty (Ubuntu texlive-science)" >&2; exit 1; }

fetch() {
  local name=$1 url=$2 digest=$3
  if [ ! -f "$tools/downloads/$name" ] || \
     ! (cd "$tools/downloads" && echo "$digest  $name" | sha256sum --check --status); then
    curl --fail --location --retry 3 "$url" --output "$tools/downloads/$name"
  fi
  (cd "$tools/downloads" && echo "$digest  $name" | sha256sum --check)
}

fetch pandoc.tar.gz \
  https://github.com/jgm/pandoc/releases/download/3.11/pandoc-3.11-linux-amd64.tar.gz \
  37edb3bbcf722f921a009941bf5874e2e0c09263226c9b4a2d980788cb062ab6
tar -xzf "$tools/downloads/pandoc.tar.gz" -C "$tools" --strip-components=1
fetch juliamono.tar.gz \
  https://github.com/cormullion/juliamono/releases/download/v0.63.2/JuliaMono-ttf.tar.gz \
  be6517295198ec5c92bdbaad42f4f6f8d83f921d80512b79f54fe036add95c0c
mkdir -p "$tools/juliamono"
tar -xzf "$tools/downloads/juliamono.tar.gz" -C "$tools/juliamono"
for face in Regular RegularItalic Bold BoldItalic; do
  cp "$tools/juliamono/JuliaMono-${face}.ttf" "$tools/texmf/fonts/truetype/juliamono/"
done
cp "$tools/juliamono/LICENSE" "$tools/texmf/fonts/truetype/juliamono/"
fetch unifont-18.0.01.otf \
  https://ftp.gnu.org/gnu/unifont/unifont-18.0.01/unifont-18.0.01.otf \
  88d0a14d4aa9a96419720b39ba5da921a59560d76d4ec7d523acc06ed85cd3c2
fetch unifont_upper-18.0.01.otf \
  https://ftp.gnu.org/gnu/unifont/unifont-18.0.01/unifont_upper-18.0.01.otf \
  472201e45a050bff4c7658208b8c73e34ebce3764b287a5a46c7774038ef9391
cp "$tools/downloads/"*.otf "$tools/texmf/fonts/opentype/unifont/"
mktexlsr "$tools/texmf"
export TEXMFHOME="$tools/texmf"
for font in JuliaMono-Regular.ttf unifont-18.0.01.otf unifont_upper-18.0.01.otf \
  texgyrepagella-regular.otf texgyrepagella-bold.otf texgyrepagella-italic.otf texgyrepagella-bolditalic.otf \
  texgyreheros-regular.otf texgyreheros-bold.otf texgyreheros-italic.otf texgyreheros-bolditalic.otf; do
  kpsewhich "$font" || { echo "Missing book font: $font" >&2; exit 1; }
done
if [ -n "${GITHUB_PATH:-}" ]; then
  echo "$tools/bin" >> "$GITHUB_PATH"
  echo "TEXMFHOME=$tools/texmf" >> "$GITHUB_ENV"
fi
