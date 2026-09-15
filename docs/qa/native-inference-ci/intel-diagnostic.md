# Intel Mac diagnostic branch only

This branch adds a 120-second engine stack-capture request to the Unix smoke
step, for manual lane `macos-intel`. A diagnostic stop fails and does not qualify
the package. Do not merge this branch-only workflow override into the candidate.
The shared stage/stack support is already on the main candidate; its normal
qualification workload and wide hang guards are unchanged. See CI-F07.
