# Targeted MSVC compile check

Branch-only CI-F11 diagnostic under the existing native-inference pipeline
plan. Do not merge this workflow into the delivery candidate.

1. Configure the existing MSVC 14.44 product with the CPU backend.
2. Compile the actual formerly failing `bridge.cpp.obj` before compiling or
   linking the engine. Confirm the SDK `small` macro and retain the compiler log.
3. A pass verifies this adapter compile repair only. Existing full CUDA builds
   can retain their useful compiler-cache population; no broad QA restart is
   selected here. CUDA delivery and complete exact-candidate gates remain open.

The source parent `935e9bbfd` changes only the private local buffer name and
retains the before/after macro control and Debug/Apple-ASan compilation proof.
This diagnostic does not implement the proposed MSVC/Vulkan packaging change.
