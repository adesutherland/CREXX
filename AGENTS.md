# Repository Instructions

- When debugging anything that can emit large or unbounded output, redirect both stdout and stderr to a temp log file created with `mktemp`, then inspect that file with focused reads such as `tail`, `sed`, or `grep`.
- Apply the temp-log workflow to verbose builds, parser-mode sessions, tracing, fixed-point loops, and any command that might otherwise flood the terminal or crash the agent session.
- Keep the terminal-facing output short: print the temp log path, then read back only the relevant slices needed for diagnosis.
