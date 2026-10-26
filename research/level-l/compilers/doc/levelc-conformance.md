# Level C conformance and known limitations

This is the current Classic Rexx research profile implemented in Level B with
Level L parsing, semantic IR and RXAS emission. The [user guide](levelc-user-guide.md)
explains its surface; [architecture](levelc-architecture.md) owns execution and
lifetime contracts. [PLAN](../PLAN.md) owns the bounded local research closure
and selected checks. This document separates implemented behavior, accepted
profile choices, known defects and unqualified capabilities. A documented defect
may remain in that local closure; it is not silently converted into conformance.

Native `rxc-c` is a comparison implementation. Maintained native sources and
checked expectations can supply independent cases; native passes or defects do
not become this compiler's results or correctness targets.

<a id="current-baseline"></a>

## Current implementation

The compiler supports the instruction families below and all 95 shared BIF
names. Static compilation emits RXAS. INTERPRET and automatic Rexx source calls
use the same frontend with the optional checked semantic-IR executor; no native
source compiler, assembler or second Classic evaluator is used at runtime.

The current static family is `crexx.c` version 24, with 97 operations and five
C types. Seven optional `crexx.c.dynamic` version 1 operations preserve those
identities. The runtime adapter has 39 exports, including the pure
`clausepending` predicate. Earlier family versions are refused. These are
implementation identities, not counts of independently qualified behaviors.

Expressions retain left-associative power, tighter prefix precedence, integer
division/remainder, strict and negated comparisons, and eager logical operands.
Shared B owns numeric normalization, rounding, LOSTDIGITS and error identities.
Source validation includes dead text; runtime character configuration does not
re-lex an already compiled unit. Retained physical lines and RXPP source maps
have the distinct inventory/provenance contracts described in architecture.

<a id="static-language-functionality"></a>

## Instruction profile

LC-I identifiers retain their reference mapping without a second progress ledger.
The table describes implemented forms; the limits below and actual independent
cases remain part of their meaning. The maintained instruction/BIF panel and
compact interactions are selected in PLAN; support or G consumer checks do not
establish complete G qualification.

| ID | Instruction | Implemented behavior / important boundary |
|---|---|---|
| LC-I-01 | SAY | Optional once-only expression and configured Unicode output; bare SAY emits one empty line. Historical raw-byte host parity is not promised. |
| LC-I-02 | DROP | Ordered direct/indirect lists, captured indirect words, live compound names and shared aliases/defaults; later words see earlier effects. |
| LC-I-03 | Assignment | Evaluate RHS once, then perform one raw-name write; compound-tail resolution follows RHS effects. |
| LC-I-04 | NOP | No runtime language operation; retain reached eligibility, clause and observation obligations. |
| LC-I-05 | OPTIONS | First complete bare-word header selects request-local source policy. Later operands retain their effects/errors; the runtime word processor currently has no work. |
| LC-I-06 | IF | Once-only exact-logical condition, nearest/contextual ELSE, lazy selected arm and explicit entry/continuation semantics. |
| LC-I-07 | SELECT | Ordered lazy WHEN tests, OTHERWISE and source-backed no-match error; no reevaluation after handler delivery. |
| LC-I-08 | DO | Counted, controlled, FOREVER, WHILE and UNTIL forms; written-order headers, live controls, END names and frame-owned state. |
| LC-I-09 | LEAVE | Nearest/named same-routine targets; close crossed owners without selected end work. Default static and reached-time STRICTC errors remain distinct. |
| LC-I-10 | ITERATE | Retain selected owner; close inner owners and perform selected UNTIL/advance/entry work in order. |
| LC-I-11 | ARG | PARSE UPPER ARG through the shared complete-template executor; raw presence and empty/omitted positions stay distinct. |
| LC-I-12 | PROCEDURE | Reached eligibility, one private pool and ordered EXPOSE against the immediate caller; normal/abrupt restoration. |
| LC-I-13 | CALL | Local/BIF/external resolution, omissions, RESULT, direct/quoted names, signed/scalar/process providers, source calls and four-condition CALL ON/OFF. |
| LC-I-14 | RETURN | Optional present/absent result, contextual continuation forms and owned completion before loop/private-pool cleanup. |
| LC-I-15 | EXIT | Optional completion on the correct program root, with checked call unwind before further operands or result use. |
| LC-I-16 | PULL | Selected queue/fallback source and UPPER template execution; bare PULL still consumes input. |
| LC-I-17 | PUSH | Optional captured text inserted at the front of the selected queue; operand transfer prevents insertion. |
| LC-I-18 | QUEUE | Optional captured text inserted at the tail of the selected queue; no private queue is invented. |
| LC-I-19 | PARSE | ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR, UPPER and ordered arbitrary templates, patterns, targets/dots and positions. EXTERNAL/NUMERIC are excluded. |
| LC-I-20 | ADDRESS | Environment selection/VALUE/swap, transient commands, WITH resources and lexical binding/writeback. |
| LC-I-21 | Implicit command | Once-only expression under the active environment, owned command lifecycle and retained non-string-head warnings. |
| LC-I-22 | NUMERIC | DIGITS/FORM/FUZZ defaults and values, shared validation, executing-frame installation and call restoration. |
| LC-I-23 | SIGNAL | Direct/quoted/VALUE same-frame transfer; seven-condition ON/OFF, causing-event state and crossed-owner cleanup. |
| LC-I-24 | TRACE | Shared option/query state, command inhibition/reporting and truthful retained source/value observations within the practical scope below. |
| LC-I-25 | INTERPRET | Once-only source, complete label-free groups, live caller state, local/compiled/IR calls, handlers and checked resumption/transfer. |

Each instruction retains its maintained cases under `tests/` and the shared
Classic runtime. The [native compliance reference](../../../../compiler/docs/levelc_compliance_reference.md)
and [compatibility worklist](../../../../docs/planning/release-1/levelc-compatibility-worklist.md)
retain detailed reference and syntax criteria. Their release-status tables are
not this compiler's status or an implicit new QA commission.

## BIF integration crosswalk

All names bind existing shared B entries through ordered operands, a separate
presence vector, current context and checked result/error transport. No BIF body,
numeric representation, word scanner, Unicode algorithm or variable pool is
reimplemented in the compiler. Every name has independent value/error/context
obligations; a catalogue entry or unreachable call alone proves admission only.

<a id="group3-bif-crosswalk"></a>

| Family | Existing owner and static integration contract | Independent fixture / boundary |
|---|---|---|
| BF01 | Shared character operations and direct BIF entries; exact text, scalar positions, padding and Classic C Latin-1 casing | `compiler/tests/rexx_src/levelc_bif_reference_audit.rexx`, `levelc_unicode_character_bifs.rexx`; Group 3 values, presence/errors and `classic-casing.crexx` with its independent all-256 checked oracle |
| BF02 | CharacterScan plus current Config; Unicode/configured blanks and word positions, no compiler splitter | The same maintained value/Unicode panels; `tests/group3/runtime-c.crexx` and `runtime-host.crexx` supply configured blanks |
| BF03 | Shared values, numeric context and fixed ordinal bridge; signed64 WHOLE remains distinct from arbitrary-precision radix operands | `levelc_bif_numeric_context.rexx`, `levelc_bif_whole.rexx`, `levelc_bif_integer_limits.rexx`, `levelc_latin1_ordinals.rexx`, `levelc_literal_ordinals.rexx`; Group 3 all-ordinal, rounding/guard and error boundaries |
| BF04 | Existing 23 adapters and pinned Unicode core; raw presence, exact codecs and Classic error transport | `levelc_unicode_services.rexx` and its checked expected output; Group 3 omission/replacement/high-scalar errors, reusing existing UCD qualification |
| BF05 | Existing live activation/pool and executing numeric frame; VALUE/SYMBOL preserve compound/default/dropped states | Maintained reference/numeric/whole/pool-read fixtures; Group 3 shared-pool/stem and current host-context boundaries |
| BF06 | Existing Config-selected queue count and acquisition owners | `levelc_bif_queued.rexx`; existing Group 2 queue selection/lifecycle proof remains owned there |
| BF07 | Activation ADDRESS/CONDITION/TRACE entries, one generated English diagnostic catalog, Config source lines and C-owned provenance | `levelc_bif_errortext.rexx`, `levelc_bif_sourceline.rexx`; Group 3 newline/NUL/host inventory and SRCMAP fixtures, including `srcmap-contracts.crexx` |
| BF08 | Existing eight Config/host stream adapters; checked positions, codecs, EOF/close and NOTREADY | Existing Unicode/stream controls and Group 3 eight-function language consumer; Group 2 retains its host transport/cleanup proof |
| BF09 | Activation clause clock and Config RNG; no process-global state substitute | Maintained reference DATE/TIME conversions and `levelc_bif_config_lifecycle.rexx`; Group 3 configured host/RNG isolation, preserving Group 1/2 clock proof |

The maintained sources/oracles are inputs, not inherited passes. Shared ABI and
lifecycle proof can cover one call route without multiplying every mode/VM/host
axis across 95 names. Stateful entries retain the owning instruction/host cases.
The user guide's [BIF reference](levelc-user-guide.md#built-in-function-reference)
describes source arguments and results.

## Runtime binding inventory

These 95 exact name/entry pairs agree with the C-owned
[`llcbifs` catalogue](../levelc/llcbifs.crexx). Every selected entry returns
RexxValue and takes mandatory first
`context_ref = reference .rexxclassicbifs..RexxBifCallContext`. ADDRESS, ARG,
CONDITION and TRACE additionally take the current activation reference, in that
order. The context retains separate operand presence/count, configuration,
pool and error state. Explicit empty and omitted values remain distinct.
Numeric wrappers preserve their declared inherited numeric settings; matching
metadata alone does not prove the caller's NUMERIC or host semantics.

| Context code | Required state beyond ordinary operands/presence/error transport |
|---|---|
| `T` | Current text configuration: exact codepoints, admitted character/case/comparison rules and padding/ranges used by the entry. No implicit normalization. |
| `W` | `T` plus configured word/blank scanning. |
| `O` | Fixed Latin-1 ordinal bridge and rejection above U+00FF for byte-valued operations; radix grouping/width remains entry-owned. |
| `N` | Caller NUMERIC state required by the entry and its argument checks; preserve inherited DIGITS/FORM/FUZZ, rounding and condition behavior where applicable. |
| `A` | Explicit second activation reference: argument state, current condition or frame ADDRESS/TRACE, according to the entry. |
| `P` | Live visible pool and text-name configuration; VALUE may mutate it or access a configured external pool. |
| `K` | Context-bound activation clause clock and shared elapsed/reset state; direct harnesses may use the existing injected pool clock. |
| `Q` | Execution-selected queue through current configuration; query must not consume it. |
| `R` | Execution/configuration-owned RNG state and explicit seed mutation. |
| `S` | Retained source inventory supplied through configuration, including exact line/count/error behavior. |
| `H` | Execution-owned stream/configuration/provider state; qualification/path policy, codec, lifecycle and NOTREADY/error transport. |
| `U` | Shared B Unicode provider with existing C text/ordinal, optional-presence and error adapters. |
| `—` | No additional state identified here; the common context and entry-specific checks still apply. ERRORTEXT uses the shared message catalog. |

Fixture abbreviations name maintained inputs with authored expected results:

- `B/Foo` means [`lib/rxfnsc/tests_functional/testRexxClassicBifFoo.crexx`](../../../../lib/rxfnsc/tests_functional/).
  `B/Numeric` and `B/Bitwise` are the existing shared harnesses;
  `B/Bifs` denotes `testRexxClassicBifs.crexx` in that directory.
- `CU` means [`compiler/tests/rexx_src/levelc_unicode_services.rexx`](../../../../compiler/tests/rexx_src/levelc_unicode_services.rexx)
  and its `.expected`, plus [`classic_unicode_contract.py`](../../../../compiler/tests/classic_unicode_contract.py)
  for ordinals/replacement omission and the independent public B
  [`ts_unicode_public.crexx`](../../../../lib/rxfnsb/unicode/tests/ts_unicode_public.crexx).
- `CS` means the stream cases of [`classic_unicode_contract.py`](../../../../compiler/tests/classic_unicode_contract.py)
  named in the [Unicode/stream crosswalk](../../../../compiler/docs/levelc_unicode_and_streams.md#regression-crosswalk),
  [`test_classic_stream_lifecycle.c`](../../../../compiler/tests/src/test_classic_stream_lifecycle.c)
  and [`classic_stream_package.rexx`](../../../../tests/rxpa/classic_stream_package.rexx).
  These provide reusable semantics/lifecycle/byte expectations, not coverage of every stream case.

Some direct B fixtures exercise retained alternate entries: ADDRESS/TRACE's pool
clients differ from the selected activation-frame entries, and Subword's direct
fixture also uses a common helper. The maintained
`compiler/tests/rexx_src/levelc_address_whole.rexx`, `levelc_trace_values.rexx` and
`levelc_bif_reference_audit.rexx` checked expectations preserve those distinctions.

| Name | Existing B runtime entry | Context | Family | Reusable fixture |
|---|---|---|---|---|
| `ABBREV` | `rexxclassicbifabbrev.rexxclassicbif_abbrev` | T | BF01 | `B/Abbrev` |
| `ABS` | `rexxclassicbifabs.rexxclassicbif_abs` | N | BF03 | `B/Abs` |
| `ADDRESS` | `rexxclassicbifaddress.rexxclassicbif_address_frame` | A | BF07 | `B/Address` |
| `ARG` | `rexxclassicbifarg.rexxclassicbif_arg` | A | BF05 | `B/Arg` |
| `B2X` | `rexxclassicbifb2x.rexxclassicbif_b2x` | — | BF03 | `B/B2x` |
| `BITAND` | `rexxclassicbifs.rexxclassicbif_bitand` | O | BF03 | `B/Bitwise` |
| `BITOR` | `rexxclassicbifs.rexxclassicbif_bitor` | O | BF03 | `B/Bitwise` |
| `BITXOR` | `rexxclassicbifs.rexxclassicbif_bitxor` | O | BF03 | `B/Bitwise` |
| `C2D` | `rexxclassicbifc2d.rexxclassicbif_c2d` | O, N | BF03 | `B/C2d` |
| `C2X` | `rexxclassicbifc2x.rexxclassicbif_c2x` | O | BF03 | `B/C2x` |
| `CENTER` | `rexxclassicbifcenter.rexxclassicbif_center` | T | BF01 | `B/Center` |
| `CENTRE` | `rexxclassicbifcenter.rexxclassicbif_centre` | T | BF01 | `B/Centre` |
| `CHANGESTR` | `rexxclassicbifchangestr.rexxclassicbif_changestr` | T | BF01 | `B/Changestr` |
| `COMPARE` | `rexxclassicbifcompare.rexxclassicbif_compare` | T | BF01 | `B/Compare` |
| `CONDITION` | `rexxclassicbifcondition.rexxclassicbif_condition` | A | BF07 | `B/Condition` |
| `COPIES` | `rexxclassicbifcopies.rexxclassicbif_copies` | T | BF01 | `B/Copies` |
| `COUNTSTR` | `rexxclassicbifcountstr.rexxclassicbif_countstr` | T | BF01 | `B/Countstr` |
| `D2C` | `rexxclassicbifd2c.rexxclassicbif_d2c` | O, N | BF03 | `B/D2c` |
| `D2X` | `rexxclassicbifd2x.rexxclassicbif_d2x` | O, N | BF03 | `B/D2x` |
| `DATATYPE` | `rexxclassicbifdatatype.rexxclassicbif_datatype` | T | BF03 | `B/Datatype` |
| `DATE` | `rexxclassicbifdate.rexxclassicbif_date` | K | BF09 | `B/Date` |
| `DELSTR` | `rexxclassicbifdelstr.rexxclassicbif_delstr` | T | BF01 | `B/Delstr` |
| `DELWORD` | `rexxclassicbifdelword.rexxclassicbif_delword` | W | BF02 | `B/Delword` |
| `DIGITS` | `rexxclassicbifnumeric.rexxclassicbif_digits` | N | BF05 | `B/Numeric` |
| `ERRORTEXT` | `rexxclassicbiferrortext.rexxclassicbif_errortext` | — | BF07 | `B/Errortext` |
| `FORM` | `rexxclassicbifnumeric.rexxclassicbif_form` | N | BF05 | `B/Numeric` |
| `FORMAT` | `rexxclassicbifformat.rexxclassicbif_format` | N | BF03 | `B/Format` |
| `FUZZ` | `rexxclassicbifnumeric.rexxclassicbif_fuzz` | N | BF05 | `B/Numeric` |
| `INSERT` | `rexxclassicbifinsert.rexxclassicbif_insert` | T | BF01 | `B/Insert` |
| `LASTPOS` | `rexxclassicbiflastpos.rexxclassicbif_lastpos` | T | BF01 | `B/Lastpos` |
| `LEFT` | `rexxclassicbifleft.rexxclassicbif_left` | T | BF01 | `B/Left` |
| `LENGTH` | `rexxclassicbiflength.rexxclassicbif_length` | T | BF01 | `B/Length` |
| `LOWER` | `rexxclassicbifs.rexxclassicbif_lower` | T | BF01 | `B/Bifs` |
| `MAX` | `rexxclassicbifmax.rexxclassicbif_max` | N | BF03 | `B/Max` |
| `MIN` | `rexxclassicbifmin.rexxclassicbif_min` | N | BF03 | `B/Min` |
| `OVERLAY` | `rexxclassicbifoverlay.rexxclassicbif_overlay` | T | BF01 | `B/Overlay` |
| `POS` | `rexxclassicbifpos.rexxclassicbif_pos` | T | BF01 | `B/Pos` |
| `QUEUED` | `rexxclassicbifqueued.rexxclassicbif_queued` | Q | BF06 | `B/Queued` |
| `RANDOM` | `rexxclassicbifrandom.rexxclassicbif_random` | R | BF09 | `B/Random` |
| `REVERSE` | `rexxclassicbifreverse.rexxclassicbif_reverse` | T | BF01 | `B/Reverse` |
| `RIGHT` | `rexxclassicbifright.rexxclassicbif_right` | T | BF01 | `B/Right` |
| `SIGN` | `rexxclassicbifsign.rexxclassicbif_sign` | N | BF03 | `B/Sign` |
| `SOURCELINE` | `rexxclassicbifsourceline.rexxclassicbif_sourceline` | S | BF07 | `B/Sourceline` |
| `SPACE` | `rexxclassicbifspace.rexxclassicbif_space` | W | BF02 | `B/Space` |
| `STRIP` | `rexxclassicbifstrip.rexxclassicbif_strip` | T | BF01 | `B/Strip` |
| `SUBSTR` | `rexxclassicbifsubstr.rexxclassicbif_substr` | T | BF01 | `B/Substr` |
| `SUBWORD` | `rexxclassicbifsubword.rexxclassicbif_subword` | W | BF02 | `B/Subword` |
| `SYMBOL` | `rexxclassicbifsymbol.rexxclassicbif_symbol` | P | BF05 | `B/Symbol` |
| `TIME` | `rexxclassicbiftime.rexxclassicbif_time` | K | BF09 | `B/Time` |
| `TRACE` | `rexxclassicbiftrace.rexxclassicbif_trace_frame` | A | BF07 | `B/Trace` |
| `TRANSLATE` | `rexxclassicbiftranslate.rexxclassicbif_translate` | T | BF01 | `B/Translate` |
| `TRUNC` | `rexxclassicbiftrunc.rexxclassicbif_trunc` | N | BF03 | `B/Trunc` |
| `UPPER` | `rexxclassicbifs.rexxclassicbif_upper` | T | BF01 | `B/Bifs` |
| `VALUE` | `rexxclassicbifvalue.rexxclassicbif_value` | P | BF05 | `B/Value` |
| `VERIFY` | `rexxclassicbifverify.rexxclassicbif_verify` | T | BF01 | `B/Verify` |
| `WORD` | `rexxclassicbifword.rexxclassicbif_word` | W | BF02 | `B/Word` |
| `WORDINDEX` | `rexxclassicbifwordindex.rexxclassicbif_wordindex` | W | BF02 | `B/Wordindex` |
| `WORDLENGTH` | `rexxclassicbifwordlength.rexxclassicbif_wordlength` | W | BF02 | `B/Wordlength` |
| `WORDPOS` | `rexxclassicbifwordpos.rexxclassicbif_wordpos` | W | BF02 | `B/Wordpos` |
| `WORDS` | `rexxclassicbifwords.rexxclassicbif_words` | W | BF02 | `B/Words` |
| `X2B` | `rexxclassicbifx2b.rexxclassicbif_x2b` | — | BF03 | `B/X2b` |
| `X2C` | `rexxclassicbifx2c.rexxclassicbif_x2c` | O | BF03 | `B/X2c` |
| `X2D` | `rexxclassicbifx2d.rexxclassicbif_x2d` | O, N | BF03 | `B/X2d` |
| `XRANGE` | `rexxclassicbifxrange.rexxclassicbif_xrange` | O | BF03 | `B/Xrange` |
| `CHARIN` | `rexxclassicbifstream.rexxclassicbif_charin` | H | BF08 | `CS` |
| `CHAROUT` | `rexxclassicbifstream.rexxclassicbif_charout` | H | BF08 | `CS` |
| `CHARS` | `rexxclassicbifstream.rexxclassicbif_chars` | H | BF08 | `CS` |
| `LINEIN` | `rexxclassicbifstream.rexxclassicbif_linein` | H | BF08 | `CS` |
| `LINEOUT` | `rexxclassicbifstream.rexxclassicbif_lineout` | H | BF08 | `CS` |
| `LINES` | `rexxclassicbifstream.rexxclassicbif_lines` | H | BF08 | `CS` |
| `QUALIFY` | `rexxclassicbifstream.rexxclassicbif_qualify` | H | BF08 | `CS` |
| `STREAM` | `rexxclassicbifstream.rexxclassicbif_stream` | H | BF08 | `CS` |
| `UNICODEVERSION` | `rexxclassicbifunicode.rexxclassicbif_unicodeversion` | U | BF04 | `CU` |
| `TONFD` | `rexxclassicbifunicode.rexxclassicbif_tonfd` | U | BF04 | `CU` |
| `TONFC` | `rexxclassicbifunicode.rexxclassicbif_tonfc` | U | BF04 | `CU` |
| `TONFKD` | `rexxclassicbifunicode.rexxclassicbif_tonfkd` | U | BF04 | `CU` |
| `TONFKC` | `rexxclassicbifunicode.rexxclassicbif_tonfkc` | U | BF04 | `CU` |
| `ISNFD` | `rexxclassicbifunicode.rexxclassicbif_isnfd` | U | BF04 | `CU` |
| `ISNFC` | `rexxclassicbifunicode.rexxclassicbif_isnfc` | U | BF04 | `CU` |
| `ISNFKD` | `rexxclassicbifunicode.rexxclassicbif_isnfkd` | U | BF04 | `CU` |
| `ISNFKC` | `rexxclassicbifunicode.rexxclassicbif_isnfkc` | U | BF04 | `CU` |
| `TOUPPERCASE` | `rexxclassicbifunicode.rexxclassicbif_touppercase` | U | BF04 | `CU` |
| `TOLOWERCASE` | `rexxclassicbifunicode.rexxclassicbif_tolowercase` | U | BF04 | `CU` |
| `TOCASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_tocasefold` | U | BF04 | `CU` |
| `TOSIMPLECASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_tosimplecasefold` | U | BF04 | `CU` |
| `TOTURKICCASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_toturkiccasefold` | U | BF04 | `CU` |
| `TOTURKICSIMPLECASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_toturkicsimplecasefold` | U | BF04 | `CU` |
| `GRAPHEMECOUNT` | `rexxclassicbifunicode.rexxclassicbif_graphemecount` | U | BF04 | `CU` |
| `GRAPHEMEREVERSE` | `rexxclassicbifunicode.rexxclassicbif_graphemereverse` | U | BF04 | `CU` |
| `GRAPHEMESUBSTR` | `rexxclassicbifunicode.rexxclassicbif_graphemesubstr` | U | BF04 | `CU` |
| `GRAPHEMEPOS` | `rexxclassicbifunicode.rexxclassicbif_graphemepos` | U | BF04 | `CU` |
| `ENCODE` | `rexxclassicbifunicode.rexxclassicbif_encode` | U | BF04 | `CU` |
| `DECODE` | `rexxclassicbifunicode.rexxclassicbif_decode` | U | BF04 | `CU` |
| `ISDECODABLE` | `rexxclassicbifunicode.rexxclassicbif_isdecodable` | U | BF04 | `CU` |
| `ISENCODINGSUPPORTED` | `rexxclassicbifunicode.rexxclassicbif_isencodingsupported` | U | BF04 | `CU` |

## Accepted profile choices

- [Unicode text](../../../../compiler/docs/levelc_unicode_and_streams.md) uses
  codepoint positions, exact text and no implicit normalization. Ordinals
  0–255 retain the fixed Latin-1 bridge; byte-valued operations reject input
  above U+00FF rather than reinterpret UTF-8 bytes. Unicode behavior above 255
  is intentional. Ordinary Classic C character and name casing keeps an input
  codepoint in 0–255 within that range, retaining the original character when a
  simple mapping would leave it. Higher codepoints use simple Unicode mappings.
  Explicit Unicode casing retains its full contract; generic B/G Unicode
  semantics are unchanged.
- Modern SAY emits Unicode text through the output route, rather than promising
  historical raw-byte output. Wider host/callback/platform proof remains in
  the [qualification boundary](#qualification-limits).
- [Static statement providers](levelc-architecture.md#call3-static-provider-contract)
  are explicit, signed and prepared by their owning B/G/C pipeline. External
  function expressions, ordinary scalar typed/native procedures and configured
  callbacks use the checked scalar interface described below. Automatic Classic
  source loading uses the optional dynamic runtime; ordinary provider calls
  retain their prepared metadata boundary. Delivery/platform qualification is separate.
- PARSE EXTERNAL and PARSE NUMERIC remain outside the current agreed source
  scope; their final release disposition is separate from this research profile.
- ADDRESS retains authored raw resource symbols. The shared 54.1 message prints
  that resource spelling, such as `missing.0`; native C can print `MISSING.0`.
  The condition identity and causing clause are unchanged.
- The approved SIGNAL profile canonicalizes direct, quoted and VALUE names
  through the shared Classic uppercase policy, then compares exact canonical text.
  This intentionally differs from reference literal/evaluated-name spelling;
  `1.20`, `1.2` and names with trailing blanks remain distinct. See the
  [language guide](../../../../docs/books/crexx_language_reference/statements.md#level-c-classic-signal)
  and [whole SIGNAL contract](levelc-architecture.md#signal-complete-contract).


<a id="calls-and-host-integration"></a>

## Calls and host boundaries

Ordinary scalar B/G/RXPA calls use declared defaults/ABI and runtime RexxValue
conversion. Exposed scalar updates use private temporaries and are discarded;
B/G globals are inaccessible. Arrays, arbitrary objects, methods/factories,
true references and task capabilities need explicit adapters.

Executable CALL shares ADDRESS's process/byte-channel capabilities, passes direct
argv, returns exit-status text and inherits host stdout/stderr. It adds no shell
parsing or redirection syntax. A source child owns a fresh external root/pool;
its EXIT does not exit its caller. Source lookup and unreadable/invalid-file
stop rules are defined in [architecture](levelc-architecture.md#rexx-source-search).

Callback pool windows are synchronous, enabled and generation-checked; they do
not grant ordinary typed calls implicit access. Real HALT ingress is execution
owned. An explicit host completion boundary owns the first escaping event after
cleanup; without it, unhandled C events retain terminal HALT with their original
tag/payload/origin. General outward raw-provider resumption is not implemented.

<a id="implemented-practical-trace"></a>

## Practical TRACE and accepted observations

<a id="practical-trace-target"></a>

The implemented contract follows the agreed **native Level C practical scope**, as
defined in [TRACE requirements](../../../../docs/ai-context/CREXX_TRACE_REQUIREMENTS.md)
and native [LC-I-24 / LC-AC-77](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps).
It does not require broader TRACE functionality for this compiler:

- Bare TRACE resets to Normal. Static/VALUE options and TRACE() share
  invocation state; the BIF returns the previous setting and applies a valid
  new one. Invalid options produce a source-anchored Classic error.
- `!` inhibits commands. `?` changes the reported interactive flag without
  prompting or reading control input. Signed counts are validated: zero selects
  Off; nonzero skip/pause and suppression counts are not enacted.
- SCAN is unsupported. Labels mode emits no label-pass records. Source,
  result and intermediate coverage is partial, including compound names,
  final expressions and optimized-away values. Emitted scalars must be their
  actual Unicode text; unavailable events are omitted.
- TRACE ENV selects Classic Error via its first letter E; the B/G environment
  extension is separate. ADDRESS classification remains coarse: N/E report
  nonzero RC or a condition; F reports negative RC or FAILURE.
- As in native C, an unexported command-only unit without TRACE, a TRACE BIF or
  an external-call boundary does not activate the shared controller at startup.
  Its initial N option state alone therefore produces no command records.
  Explicit Normal or bare TRACE activates the existing command reporting.

This compiler omits the native text `>  >` file-transition marker; LLM
records retain logical filenames. A multiline composite clause can produce
additional truthful source records. These presentation differences and the
partial event coverage above are accepted within this practical scope.
Retained records can also include exact BIF results or static TRACE operands
omitted by native transcripts. Authored PARSE/ARG/PULL field observations use
the same shared owners.
Incorrect displayed values, changed ordinary behavior, wrong targets/results,
or broken state, ownership, error and resource semantics remain material defects.


CALL ON/OFF handles ERROR, FAILURE, HALT and NOTREADY. Handlers receive fresh
zero-argument CALL/DELAY state; normal return results are ignored. Ordinary SIGL
updates while reserved .SIGL is preserved. SIGNAL transfers in the same frame.
Clause completion captures decisions, headers and results before delivery.
A source-less BIF handler leaves completion with the interrupted dispatcher;
there is no independent child SIGNAL landing or arbitrary provider resumption.

Clock-free clauses remain clock-free; interrupted samples survive handlers.
PARSE/ARG/PULL observations report actual retained fields/dots after their
individual effects, including earlier writes before a later failure. Direct
library consumers default to no observations; LLM coverage remains partial.
Dynamic literal-only IR constants omit intermediate events. A dropped DO-control
read during advance is attributed to END here; native helper diagnostics can
use the DO header. Internal helper raw-location presentation can differ while
SIGL, condition state and required errors remain material.

## Implementation limits

These are explicit implemented boundaries, not claims that the corresponding
universal language or host capability has been qualified:

- INTERPRET accepts complete label-free groups. Caller labels/routines remain
  available; outer loops are not generated LEAVE/ITERATE targets.
- Raw multi-function IR with immutable CALL ON declarations cannot make an
  internal call across its source-owned policy domain. Ordinary source shares
  one body; no general foreign policy domain is inferred.
- `scope.configureSource(enabled, roots)` is the runtime source-policy boundary.
  Standalone scopes start enabled; a cwd snapshot alone does not establish an
  embedding permission policy. Embedded policy integration is unqualified.
- Source lookup does not implicitly search OS PATH/REXXPATH/import roots or infer
  an executable ABI from extensions, shebangs or execute bits. Empty/NUL names
  have no filesystem candidate; process arguments retain OS NUL restrictions.
- Compiler and runtime work/storage limits are separate. Dynamic defaults and
  overflow/refusal rules are in the [architecture limits](levelc-architecture.md#implementation-interfaces-and-limits).
  They are logical accounting bounds, not an RSS ceiling or bound on hostile host code.
- Full G/B languages, proper B self-hosting, RexxScript migration and default
  compiler replacement are separate programmes; existing consumers remain intact.

## Known defects and uncertainty

A **known defect** is a reproduced mismatch with an independent expected value,
control/state/ownership/error or required observation. It may remain documented
for this bounded research closure, with its source case, expected/actual result,
affected mechanism and owner/next action. A suspected issue stays labelled as
suspected until reproduced. Missing platform evidence is unqualified capability,
and the accepted profile choices above are not relabelled as passing reference
conformance. Historical repaired bugs are not a permanent run-history list.

No unresolved, reproduced C correctness defect is currently recorded for this
profile and its maintained panel. Record new findings here against their
independent expectations; unrelated repaired core defects are not C limitations.
Untested combinations remain unqualified.

<a id="reusable-level-l-backend-boundary"></a>
<a id="material-work-remaining"></a>

## Qualification limits

PLAN selects the Classic instruction/BIF panel and compact interaction checks
for local research closure. A local commit or successful selected cell does not
establish universal Rexx conformance, full per-name/host combinations, production
performance, installed/relocated delivery, hosted/declared platforms or release
readiness. No broad platform, sanitizer or performance campaign is commissioned
by this document.

Existing first-party sanitizer findings and supported-platform closure remain
owned by [SANITIZER-WORKLIST](../../../../docs/SANITIZER-WORKLIST.md) and the
[maintained sanitizer guide](../../../../docs/ai-context/CREXX_ASAN_TESTING.md).
This C closure neither closes those items nor claims sanitizer-clean status.
Shared L SDK, B/G, metadata/editor and constrained-platform obligations retain
their separate owners and unqualified outcomes in PLAN and the shared guides.
The implemented optional SDK/package boundary is described in the
[architecture](levelc-architecture.md#analysis-and-rxas-selection); it does not
select new G work or a production compiler replacement.
