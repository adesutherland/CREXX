# Retained performance evidence

Each dated directory is one evidence bundle: provenance and commands, raw
samples, derived summaries, correctness results and bounded interpretation.
The directory name identifies the capture date and purpose; reruns with changed
source, host, build, lifecycle or image mode receive a new bundle rather than
overwriting the old evidence.

Raw CSV is append-only evidence once reviewed. Corrections belong in the
bundle README with a reason, or in a replacement bundle that points back to the
superseded capture.
