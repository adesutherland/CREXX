# Level C Unicode and stream contract

Adrian approved these principles and requested implementation on 2026-10-09.
The authoritative plan and qualification status are
[LC-UNICODE](../../docs/planning/release-1/levelc-compatibility-worklist.md#lc-unicode--compatible-byte-ordinals-unicode-and-encoded-streams-2026-10-09).
This document records the implemented contract. All eight criteria for this
approved local programme are verified in the
[historical development receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/levelc-unicode-2026-10-09/README.md); wider
Level C/Release 1 obligations retain their separate status.
The later ordinary-case Latin-1 boundary below belongs to the
[Classic conformance profile](../../research/level-l/compilers/doc/levelc-conformance.md)
and its maintained controls; the earlier receipt does not qualify that change.

## Ordinary values and byte compatibility

Level C holds valid Unicode text. A byte ordinal `00`–`FF` corresponds exactly
to U+0000–U+00FF, including embedded NUL. Every ordinal occupies one character
position, irrespective of its internal UTF-8 storage size. Hex/binary literals,
X2C/D2C, C2X/C2D and bitwise operations use this fixed bridge. An external
codepage never changes those ordinals. Byte-only input above U+00FF raises
Classic SYNTAX 23.1; truncation and interpreting the UTF-8 representation as
Classic bytes are prohibited.

LENGTH and ordinary substring, search, reverse, table TRANSLATE and PARSE
positions count codepoints. Assignment, concatenation and comparisons preserve
exact text. For example, `é` and `e` followed by U+0301 remain distinct. Neither
an ordinary comparison nor a codec invokes NFC.

Symbols admit the source scanner's Unicode letter categories, with its existing
ASCII digits and symbol punctuation. Runtime SYMBOL/DATATYPE and compiled pool
lookup use that same letter authority. This is not a general UAX #31 identifier
profile: combining marks or emoji are not newly admitted identifier characters.
Ordinary Classic case mapping identifies compiled names and implements
UPPER/LOWER, default TRANSLATE and PARSE UPPER (including ARG/PULL) without
expanding a scalar into several scalars. For an input in U+0000–U+00FF, a simple
mapping is applied only if its result also lies in U+0000–U+00FF; otherwise the
original scalar is retained. Thus ordinary UPPER preserves both `µ` and `ÿ`.
An input above U+00FF uses the pinned simple Unicode mapping, including mappings
back into Latin-1: ordinary LOWER maps `Ÿ` to `ÿ`. This input-domain rule does
not require upper/lower reversibility, and ordinary canonical names `ÿ` and `Ÿ`
remain distinct. LENGTH and all character positions continue to count
codepoints, preserving every Latin-1 count and the fixed byte-ordinal bridge.

The Classic C compiler and B-implemented Classic runtime obtain these mappings
from the single `cmake/CrexxClassicUnicode.cmake` generator, including mappings above Latin-1
that change UTF-8 width. Its letter-category data and lexer rules are unchanged.
Explicit Unicode TOUPPERCASE/TOLOWERCASE and the folding functions keep their
full Unicode contracts; TOUPPERCASE can therefore map `ÿ` to `Ÿ` and expand
`ß` to `SS`. Ordinary Level B/G casing and `rxunicode` are unchanged; the policy
is confined to Classic Level C. It is also the agreed future Level O rule,
without adding a Level O implementation here. BYTE-profile direct clients and
RexxScript retain their existing limited case and name policy.

Existing Unicode White_Space word/PARSE defaults and configured extra blanks
remain in force. Padding counts characters. Ordinary nonnumeric ordering follows
codepoint order, preserving ordinal order throughout U+0000–U+00FF. These
Unicode extensions must be named when claiming parity with another processor.

## Unicode runtime ownership

The 23 existing `rxunicode` procedures live in the separately selectable B
`rxunicode-core.rxbin`. `rxfnsc` stages that exact metadata and keeps its
Classic BIF and stream bodies unchanged. The standalone C runtime can select
core without G. Existing combined products select G's aggregate, which embeds
the same B members; linking both provider aggregates is invalid. G retains
its collection/iterator classes. This packaging split does not change byte
ordinals, source casing, codec selection, argument presence or error mapping.
The [shared contract](../../lib/rxfnsb/unicode/README.md#public-b-interface)
documents ownership and the acyclic minimal-B → L → optional-core build order.

## Explicit Unicode BIFs

The Classic adapters use the existing Unicode 17.0.0 rxunicode algorithms and
prepared constants. They preserve Classic omission, argument and authored
SYNTAX reporting. The public functions are:

| Family | Functions |
| --- | --- |
| Version | UNICODEVERSION |
| Normalization | TONFD, TONFC, TONFKD, TONFKC; ISNFD, ISNFC, ISNFKD, ISNFKC |
| Full casing | TOUPPERCASE, TOLOWERCASE |
| Folding | TOCASEFOLD, TOSIMPLECASEFOLD, TOTURKICCASEFOLD, TOTURKICSIMPLECASEFOLD |
| Extended graphemes | GRAPHEMECOUNT, GRAPHEMESUBSTR, GRAPHEMEPOS, GRAPHEMEREVERSE |
| Codecs | ENCODE, DECODE, ISDECODABLE, ISENCODINGSUPPORTED |

ENCODE returns an ordinary Classic string of byte ordinals. DECODE consumes
such a string and returns Unicode text. UTF-8 is the default codec. Optional
third arguments explicitly request replacement: ENCODE takes replacement byte
ordinals valid in the target encoding, while DECODE takes replacement text.
An unsupported encoding or invalid option is SYNTAX; strict conversion errors
are SYNTAX 23.1. Algorithm details and independent conformance oracles remain in
[the Unicode context](../../docs/ai-context/CREXX_UNICODE.md) and
[the typed library reference](../../docs/books/crexx_library_reference/unicode.md).

## Streams and external codepages

CHARIN, CHAROUT, CHARS, LINEIN, LINEOUT, LINES, QUALIFY and STREAM share one
execution/configuration-owned service. Internal Classic calls share that
configuration. Distinct root configurations and VM sessions own independent
handles, positions and codec selection. Explicit CLOSE closes aliases; native
payload finalization closes unclosed named resources at teardown.

Named streams are raw ordinals unless OPEN selects an encoding. Implicit default
console streams preserve the existing UTF-8 text behavior. Explicit OPEN on the
empty stream name can select BYTE or another encoding; READ and WRITE select
independent default input and output state. STREAM on the empty name queries
and closes the direction selected by its last explicit OPEN (input initially);
READ/WRITE POSITION queries address their respective direction. SAY/PULL and queued values retain
their existing console/queue transport. Source files remain Unicode UTF-8;
STREAM changes neither source decoding nor byte BIFs.

```rexx
options levelc
say stream('report.txt','C','OPEN READ ENCODING Windows-1252 CODEPOINTS')
text = linein('report.txt')
say length(text)
say stream('report.txt','C','QUERY ENCODING NAME')
say stream('report.txt','C','CLOSE')
```

Supported encodings are UTF-8, UTF-16LE/BE, UTF-32LE/BE, US-ASCII,
ISO-8859-1, Windows-1252, IBM437, IBM850 and IBM1047, with the aliases accepted
by rxunicode. BYTE/BINARY/RAW select ordinal I/O. Compact mappings are used
where the codepage actually defines them. Windows-1252 byte `80` decodes to
U+20AC, so LENGTH is one and C2X signals; ENCODE(text,'Windows-1252') recovers
ordinal `80`. No surrogate/code-unit length is exposed.

OPEN accepts READ, WRITE, BOTH, REPLACE, APPEND, ENCODING followed by a name,
CODEPOINTS, and ERROR followed by SYNTAX or REPLACE. WRITE/BOTH initially append;
REPLACE explicitly truncates. Codecs add no BOM and preserve a decoded U+FEFF.
Strict conversion is the default. ERROR REPLACE inserts U+FFFD for malformed
input and target-encoded `?` for unrepresentable output. It applies no NFC and
no grapheme counting. Unsupported commands/options receive Classic SYNTAX.

STREAM's S/D forms report state/description. Its C form implements OPEN/CLOSE
and QUERY ENCODING NAME/TARGET/ERROR/LASTERROR, READ POSITION, WRITE POSITION,
STREAMTYPE and EXISTS. EXISTS checks the host file even before it is opened.
ENCODING follows [TUTOR's stream vocabulary](https://rexx.epbcn.com/TUTOR/doc/stream/).
It is a cREXX extension through the implementation-defined command point in
[the Classic specification, A.5.8](https://www.rexxla.org/rexxlang/standards/j18pub.pdf).
It is not a universally standardized Classic command string.

Persistent streams have independent one-based character read/write positions;
LINEIN/LINEOUT start arguments are one-based line numbers. CHARS/LINES report
remaining characters/lines from the read position. N (default) and C return
that count for persistent files; I returns an indicator. Transient streams
return availability. Omitted output text with a position changes only the
write position; without either it closes the stream. CHAROUT overwrites scalars
and preserves the following scalars, including when encoded byte widths change.
A gap before output is filled with U+0000. LINEIN removes LF and an immediately
preceding CR; an unterminated final line is retained. LINEOUT writes an encoded
LF. A standalone CR remains data. Raw mode uses the same rules on ordinals.

Persistent conversion validates the current whole file before changing logical
positions or writing. Consequently malformed bytes anywhere in the current file
can fail a read. Encoded overwrite loads/decodes/re-encodes the file; it is a
correctness implementation with whole-file memory cost, not a streaming codec
or a transactional filesystem write. No hidden allocation ceiling is imposed.
Explicit transient positioning raises SYNTAX 40.42. LINEIN counts above one
raise 40.39. NUL-bearing names raise 40.27. Transient input consumes records; a failed conversion can consume the failing
record. Explicit transient positioning fails. CHARS/LINES on a transient stream
are availability indicators rather than promises of an exact future count.

Stream BIFs record EOF and OS failures as NOTREADY. Disabled traps return the
ordinary partial read or unwritten count; enabled SIGNAL/CALL traps use the existing Classic
source/condition machinery. Conversion errors take SYNTAX precedence. Invalid
NUL-bearing path arguments are rejected with exact-length validation. QUALIFY
uses the existing host filesystem qualification service.

Queue-empty PULL and PARSE PULL acquire default text through the shared Config
service. An exhausted configured input returns empty text on repeated reads;
other stream failures and configured PULL/LINEIN hook conditions retain their
existing NOTREADY transport. `RexxClassicStream.readEof()` reports only the most
recent character/line read outcome: a positive-count exhausted read with a ready
native transport. Each read and close resets that observation, so a zero-count
read or a handle closed through an alias cannot be mistaken for ordinary EOF.

## Regression crosswalk

| Contract | Permanent evidence |
| --- | --- |
| All 256 bytes, NUL, literals, PARSE positions, conversions and bitwise operations | `classic_unicode_contract.py`: ordinals and byte_error cases |
| Unicode letters, simple mappings that shrink/grow UTF-8, pool/SIGNAL/CALL parity | `levelc_unicode_symbols.rexx`, case_width |
| Ordinary Latin-1 casing stays within 0–255; higher simple mappings and explicit full casing remain distinct | `research/level-l/compilers/tests/group3/check-group3.py`: all-256 ordinary upper/lower oracle and selected name/PARSE/explicit-Unicode controls |
| Exact text, codepoint operations and explicit algorithms | `levelc_unicode_services.rexx`, ordinals, replacement_omission |
| Eight stream BIFs, raw bytes, positions/counts/close and host qualification | streams |
| Every codec, codepage mappings above U+00FF, strict/replacement and default input | codec_*, strict_stream, replacement_stream, default_transport, parse_linein |
| Authored conversion errors and NOTREADY SIGNAL/CALL | byte_error_*, untrapped, notready, call_notready |
| Configured BIN/HEX validator-consumer parity | radix_validator_parity |
| Unicode routine source/import/linked lookup | unicode_import |
| Invalid imported constant diagnostics instead of the exposed compiler crash | invalid_import |
| Independent live VM sessions, native receiver ownership and final handle counts | classic_stream_lifecycle |
| Extracted core ZIP and relocated native encoded streams | `scripts/package-core-release.py`: `classic_stream_package.rexx`, decoded text and exact Windows-1252 bytes |

The same contract runs optimized/no-opt and direct/linked. The plan owns native
ownership/isolation, static/installed packaging, focused Debug/Release/ASan
measurements and the normal product suite; this crosswalk does not substitute
for those retained receipts.
