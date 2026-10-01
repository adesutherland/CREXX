set -eu
model_dir="/tmp/crexx-ni-s6.zq0rk0ug/fresh models"
mkdir -p "$model_dir"
model_sha256() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | cut -d ' ' -f 1
  else
    shasum -a 256 "$1" | cut -d ' ' -f 1
  fi
}
fetch_model() {
  model_file="$model_dir/$1"
  expected_sha="$2"
  model_url="$3"
  if [ ! -f "$model_file" ]; then
    curl --fail --location --retry 3 --output "$model_file.part" "$model_url"
    [ "$(model_sha256 "$model_file.part")" = "$expected_sha" ] || {
      echo "SHA-256 mismatch: $model_file.part" >&2; return 1;
    }
    mv "$model_file.part" "$model_file"
  fi
  [ "$(model_sha256 "$model_file")" = "$expected_sha" ] || {
    echo "SHA-256 mismatch: $model_file" >&2; return 1;
  }
  echo "Verified: $model_file"
}
fetch_model smollm2-360m-instruct-q8_0.gguf \
  48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201 \
  'https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct-GGUF/resolve/593b5a2e04c8f3e4ee880263f93e0bd2901ad47f/smollm2-360m-instruct-q8_0.gguf'
