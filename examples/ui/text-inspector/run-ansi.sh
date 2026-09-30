#!/bin/sh
# Run the RXPP-generated ANSI launcher with its explicit bytecode dependencies.
set -eu
ui_build=${1:-cmake-build-debug}
exec "$ui_build/bin/rxvm" \
  "$ui_build/examples/ui/text-inspector/text_inspector_ansi" \
  "$ui_build/examples/ui/text-inspector/text_inspector" \
  "$ui_build/bin/ui_ansi" \
  "$ui_build/bin/ui_dialogs" \
  "$ui_build/bin/ui_terminal_view" \
  "$ui_build/bin/ui_local_resources" \
  "$ui_build/bin/ui_contract" \
  "$ui_build/bin/ui_catalog" \
  "$ui_build/bin/ui" \
  "$ui_build/bin/library"
