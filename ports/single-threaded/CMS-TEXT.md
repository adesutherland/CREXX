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
The runtime owns CMS records, supported code pages and I/O error reporting.

RXC accepts `-E encoding` for source/imports and assembly output; RXAS accepts
it for assembly input. Selection occurs before files are opened and applies
to all text in the invocation. `-e` is an alias. The candidate CMS runtime
defaults to IBM1047 logical records and also supports UTF8 byte streams with
explicit LF. `-E IBM1047` and `-E UTF8` select these explicitly.

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
with an injected non-seekable, short-read stream. A deliberately artificial
TEST-XOR encoding proves conversion before lexical analysis, including source
imports. Default UTF-8, explicit UTF8, adapter UTF8 and converted runs must
produce identical decoded assembly, bytecode, linked image and execution in
optimized and unoptimized modes. Read, conversion, write and close failures
must fail the tool; binary files must never enter the text hook.

This test adapter is neither an IBM1047 mapping nor a CMS service emulator.
It is isolated to test executables and is never linked into the product.

The lab runtime has host mapping,
record and error controls plus a small CMS 20 guest fixture; actual CMS RXC/RXAS
source/import tests and historical 24-bit integration remain open. This document
does not claim a published CMS package or full Unicode console support.
