#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
output=${1:-"$repo_root/docs/books/crexx_library_reference/classlib-api.tex"}

if command -v rexxApiDoc >/dev/null 2>&1; then
  generate() {
    rexxApiDoc "$@"
  }
elif [ -x "$repo_root/cmake-build-debug/bin/crexx" ]; then
  apidoc_tmp=$(mktemp -d "${TMPDIR:-/tmp}/crexx-apidoc.XXXXXX")
  cleanup_apidoc() {
    rm -rf "$apidoc_tmp"
  }
  trap cleanup_apidoc EXIT HUP INT TERM
  cp "$repo_root/tools/rexxDoc/rexxApiDoc.crexx" "$apidoc_tmp/rexxApiDoc.crexx"
  "$repo_root/cmake-build-debug/bin/crexx" -noexec -keep \
    "$apidoc_tmp/rexxApiDoc.crexx"
  generate() {
    "$repo_root/cmake-build-debug/bin/rxbvm" \
      "$repo_root/cmake-build-debug/bin/library.rxbin" \
      "$apidoc_tmp/rexxApiDoc.rxbin" -a "$@"
  }
else
  echo "genApiDoc.sh: build rexxApiDoc or cmake-build-debug/bin/crexx first" >&2
  exit 1
fi

: > "$output"
cd "$script_dir"

# Classes found outside classlib.
generate ../rxfnsb/rexx/stem.crexx --exposed               >> "$output"
generate ../rxfnsb/rexx/rxjson.crexx --exposed             >> "$output"
generate ../rxfnsg/rexx/http.crexx --exposed               >> "$output"
generate ../rxfnsg/rexx/httpserver.crexx --exposed         >> "$output"

# Public multi-class concurrency surface; omit implementation classes.
generate Concurrency.crexx --exposed                        >> "$output"

# Classes from classlib.
generate ArrayList.crexx                                    >> "$output"
generate ArrayListIterator.crexx                            >> "$output"
generate Bag.crexx                                          >> "$output"
generate ArrayBag.crexx                                     >> "$output"
generate ArrayBagIterator.crexx                             >> "$output"
generate Comparator.crexx                                   >> "$output"
generate Iterable.crexx                                     >> "$output"
generate Iterator.crexx                                     >> "$output"
generate ObjectLinkedList.crexx                             >> "$output"
generate TreeMap.crexx                                      >> "$output"
generate TreeMapIterator.crexx                              >> "$output"
generate Printable.crexx                                    >> "$output"
generate ObjectStack.crexx                                  >> "$output"
generate Os.crexx                                           >> "$output"
generate Qfind.crexx                                        >> "$output"
generate Rexx.crexx                                         >> "$output"
generate RexxComparator.crexx                               >> "$output"
generate Scanlex.crexx                                      >> "$output"
generate StringArrayList.crexx                              >> "$output"
generate StringArrayListIterator.crexx                      >> "$output"
generate StringHashMap.crexx                                >> "$output"
generate StringHashMapIterator.crexx                        >> "$output"
generate StringHashSet.crexx                                >> "$output"
generate StringHashSetIterator.crexx                        >> "$output"
generate StringIterable.crexx                               >> "$output"
generate StringIterator.crexx                               >> "$output"
generate StringLinkedList.crexx                             >> "$output"
generate StringObjectHashMap.crexx                          >> "$output"
generate StringObjectHashMapIterator.crexx                  >> "$output"
generate StringObjectTreeMap.crexx                          >> "$output"
generate StringObjectTreeMapIterator.crexx                  >> "$output"
generate StringOldTreeMap.crexx                             >> "$output"
generate StringStack.crexx                                  >> "$output"
generate StringTreeMap.crexx                                >> "$output"
generate StringTreeMapIterator.crexx                        >> "$output"
generate StringTreeSet.crexx                                >> "$output"
generate StringTreeSetIterator.crexx                        >> "$output"
generate Id.crexx                                           >> "$output"
generate KeyDB.crexx                                        >> "$output"
generate DateTime.crexx                                     >> "$output"
generate ISO8601.crexx                                      >> "$output"
generate OffsetDateTime.crexx                               >> "$output"
