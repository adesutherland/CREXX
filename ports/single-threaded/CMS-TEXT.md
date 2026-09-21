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
aliases, and rejects unsupported encodings. RXC checks output stream and close
errors before reporting success. The adapter must reject malformed or
unrepresentable text without silently substituting characters.

Native default/UTF8 assembly and bytecode comparisons pass, as do CLI invalid-
option controls and platform selection tests. The lab runtime has host mapping,
record and error controls plus a small CMS 20 guest fixture; actual CMS RXC/RXAS
source/import tests and historical 24-bit integration remain open. This document
does not claim a published CMS package or full Unicode console support.
