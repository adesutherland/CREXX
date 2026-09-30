# Optional CMS source and assembly text adapter

This is a local upstream-review candidate. The single-threaded VM options do
not select a character encoding. cREXX strings and compiler/assembler buffers
retain UTF-8, and RXBIN remains binary.

An explicit CMS ELF platform can define `CREXX_CMS_TEXT_IO` and provide:

```c
FILE *crexx_cms_text_open(const char *path, const char *mode);
int crexx_cms_text_encoding(const char *encoding);
```

`openfile` routes text modes through that adapter. Modes containing `b` keep
ordinary fopen. Conversion therefore occurs before source buffering and lexical
analysis, and after assembly emission. No lexer or grammar changes are implied.
The runtime owns CMS records and I/O error reporting. With the native raw libc
contract, it supplies IBM1047 record bytes and ASCII LF delimiters; cREXX's
shared codec owns character conversion at the file boundary.

RXC accepts `-E encoding` for source/imports and assembly output; RXAS accepts
it for assembly input. Selection occurs before files are opened and applies
to externally encoded files in the invocation. `-e` is an alias. IBM1047
defaults to logical records; exchange encodings use byte streams with explicit
LF. `-E IBM1047` and `-E UTF8` select these explicitly.

The native entry points call the canonical SDK
`mainframe_set_text_conversion(0)` before I/O. The SDK setter's default remains
unchanged. In a UTF VM, READLINE and text reads from special `stdin` decode
native IBM1047 into UTF-8. SAY, text writes to `stdout`/`stderr`, first-party
tool diagnostics and bounded exhaustion diagnostics encode native IBM1047.
This console encoding is independent of `-E`. Unrepresentable ordinary console
text returns an error from its output helper. Default SAY/SAYX and UTF console
FWRITE/FWRITECDPT propagate conversion failure as `UNICODE_ERROR`; output/flush
failure raises `NOTREADY`. Custom SAY callbacks keep their void ABI and own
their output policy. Exhaustion reporting retains its emergency replacement
policy. Classic BYTE line/codepoint reads and byte
or binary instructions retain raw bytes.

The existing native libc text wrapper supports sequential `r`, `w` and `a`
modes. Exchange append uses binary storage underneath its selected codec.
Update modes containing `+` are rejected before opening, so `w+` cannot truncate
a file and return a silently one-way wrapper. Text wrappers do not supply
seeking. Binary modes bypass the text wrapper and keep the underlying runtime's
mode and byte semantics.

Default desktop behavior is unchanged. It accepts UTF8/UTF-8 and lowercase
aliases, and rejects unsupported encodings. Select an encoding before opening
files; the adapter owns its invocation-wide selection. RXC checks output
stream and close errors before reporting success. RXC source/header/import
reads and RXAS input reads also reject a failed close. The adapter must report
malformed or unrepresentable text through the standard stream error/close
contracts without silently substituting characters.

Source streams need not support seeking. `file2buf` reads such a stream from
its current position through EOF, appends two zero scanner sentinels (including
empty input), and rejects read errors. Seekable files retain the existing
rewind-and-read behavior. The caller owns the returned buffer and closing the
stream; closing can still report a deferred conversion or I/O error.

On macOS/Linux, build `check_cms_text` or prepare and run the
`platform_cms_text` CTest. It uses the real RXC/RXAS entry points, RXLink and VM,
with an injected non-seekable, short-read raw stream. Native IBM1047 fixtures
use the product's shared codec through a small C fixture tool, proving conversion
before lexical analysis, including source imports. Default UTF-8, explicit UTF8,
adapter UTF8 and converted runs must
produce identical decoded assembly, bytecode, linked image and execution in
optimized and unoptimized modes. Read, conversion, write and close failures
must fail the tool; binary files must never enter the text hook.

This test adapter simulates the raw libc contract and does not emulate CMS
services. Its entry points reuse desktop frontend libraries, so diagnostic
assertions retain both desktop and native views; that fixture alone does not
prove complete native-tool console output. It is isolated to test executables
and is never linked into the product.

On macOS, `mainframe_stdio` checks native stdin and stdout/stderr, formatted
diagnostic returns, file-encoding independence, append, all 256 binary bytes,
and malformed/unrepresentable stream errors. `mainframe_handlers_utf8` and
`mainframe_handlers_byte` compile the real READLINE/FREADLINE/FREADCDPT bodies
against a small value fixture and check native stdin in both modes. They test
handler wiring without simulating the entire VM or its signal transport.
`mainframe_console_signals` executes the real switch VM with the host native
console adapter, proving terminal and caught conversion signals, native accent
bytes, custom SAY policy and write/flush errors. It reuses the desktop core
and is not a native mainframe VM build.
`oom_mainframe` checks native exhaustion diagnostics with short writes and
interruption while preserving the caller's errno. These are host component
proofs; none establishes guest qualification.

The lab runtime has host mapping,
record and error controls plus a small CMS 20 guest fixture; actual CMS RXC/RXAS
source/import tests and historical 24-bit integration remain open. This document
does not claim a published CMS package or full Unicode console support.
