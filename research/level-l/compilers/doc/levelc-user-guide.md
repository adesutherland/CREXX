# Classic Rexx compiler user guide (Level C)

<a id="level-c-user-guide"></a>

Use `llrxc-c` to compile Classic Rexx source to RXAS, then assemble, link and run
it on the cREXX VM. This guide covers that workflow, the language you can write,
its built-in functions and the compatibility rules to consider when porting a
program. INTERPRET and calls to Rexx source files use an additional runtime image.

The compiler is experimental and is written in Level B using Level L language
tools. Its research acceptance uses the existing Classic instruction/BIF panel
and compact interaction checks, with known defects and limitations recorded in
[conformance](levelc-conformance.md). This is not installed/platform, sanitizer,
release or production-replacement qualification. `llrxc-c` explicitly selects this
implementation. The separate native
`rxc-c` command is not an interchangeable spelling for it. Put `options levelc`
at the start of your source to select the language.

Start with [build and installation](#building-and-installing) and the
[first program](#compile-assemble-link-and-run). The language sections introduce
[values](#writing-classic-source), [control flow](#choosing-and-repeating-work),
[routines](#calling-other-routines), [PARSE and queues](#parsing-text-and-using-queues),
[host commands](#address-and-embedding) and
[runtime source](#interpret-and-rexx-source-calls). The
[built-in reference](#built-in-function-reference) groups every supported name by
what it does. Read [compatibility](#conformance-and-compatibility) when porting
another Rexx program. Compiler internals belong in the single
[Level C architecture](levelc-architecture.md).

## Building and installing

Use a working cREXX source checkout with CMake 3.24 or later, its normal native
C build toolchain, and Python 3 for the research build/check tooling. Native
Level B is built as the seed that compiles these B-written tools; the project
stages prepare the required libraries and generated language tables. Follow the
[repository build instructions](../../../../README.md) for platform setup.
The commands below use a POSIX shell and the existing normal Debug build tree.

From the repository root, select the C research product:

```sh
cmake -S . -B cmake-build-debug \
  -DCREXX_BUILD_RESEARCH_COMPILERS=ON \
  -DCREXX_RESEARCH_LANGUAGES=C
cmake --build cmake-build-debug --target stage-product stage-research-compilers --parallel 8
```

`stage-product` prepares ordinary tools/runtime and the Level L SDK.
`stage-research-compilers` prepares the selected compiler, target adapter and
dynamic support. The research products are in
`cmake-build-debug/research/level-l/compilers/products/`; ordinary tools and
runtime images are in `cmake-build-debug/bin/`. The default language selection
is `C;G`; selecting `C` avoids the separate G research product.

For the simplest command-line setup, install a complete private prefix:

```sh
cmake --install cmake-build-debug --prefix '/tmp/crexx review'
cmake --install cmake-build-debug --prefix '/tmp/crexx review' --component ResearchCompilers
```

Both installations copy prepared outputs; they do not build missing products.
The base installation supplies commands, ordinary runtime and native providers;
the opt-in ResearchCompilers component adds `llrxc-c`, `llccompiler.rxbin`,
`llctarget.rxbin` and `llcdynamic.rxbin`. Keep those files together with their
base installation. A program using native services is not distributed simply
by copying its one RXBIN file.

## Compile, assemble, link and run

Create `hello.rexx` in a working directory:

```rexx
options levelc
name = 'world'
say 'Hello,' name
say 01
```

Set the installation directory, then run the four stages:

```sh
crexx_bin='/tmp/crexx review/bin'
"$crexx_bin/llrxc-c" -i "$crexx_bin" -o hello.rxas hello.rexx
"$crexx_bin/rxas" hello.rxas
"$crexx_bin/rxlink" -o hello-linked hello.rxbin \
  "$crexx_bin/library.rxbin" "$crexx_bin/llctarget.rxbin" \
  "$crexx_bin/rxfnsc.rxbin" "$crexx_bin/classlib.rxbin" \
  "$crexx_bin/rxunicode-core.rxbin"
"$crexx_bin/rxbvm" --no-autoload hello-linked.rxbin
```

The result is:

```text
Hello, world
01
```

The blank between the quoted greeting and `name` inserts one blank in the
result. `01` retains its text until an operation requires numeric behavior.

Compilation writes readable RXAS and uses `-i` to find target metadata.
Assembly writes `hello.rxbin`. Linking adds the runtime implementations;
metadata selection alone does not link a body. `--no-autoload` makes the explicit
bytecode inputs sufficient rather than relying on automatic bytecode loading.
Required native providers still follow their installed contracts. Keep source
metadata for useful diagnostics; `rxlink -s` deliberately strips it.

Pass arguments after the VM's `-a` separator:

```sh
"$crexx_bin/rxbvm" --no-autoload hello-linked.rxbin -a first 'second argument'
```

Use `parse arg first, second` in the program to read those two positions. The
commas separate arguments; `parse arg first second` instead splits the first
argument into fields. Source suffixes are not significant to an explicit
`llrxc-c` request; automatic source calls have their own search policy.

### Using prepared build-tree products

Installation is convenient because it puts the target metadata in one place.
For development you can use the prepared launcher and tools directly. Create a
small metadata/runtime directory in your working area, supplying the same files:

```sh
crexx_build='/absolute/path/to/CREXX/cmake-build-debug'
crexx_products="$crexx_build/research/level-l/compilers/products"
mkdir -p c-runtime
cp "$crexx_build/bin/library.rxbin" "$crexx_build/bin/rxfnsc.rxbin" \
   "$crexx_build/bin/classlib.rxbin" "$crexx_build/bin/rxunicode-core.rxbin" c-runtime/
cp "$crexx_products/llctarget.rxbin" c-runtime/
"$crexx_products/llrxc-c" -i c-runtime -o hello.rxas hello.rexx
"$crexx_build/bin/rxas" hello.rxas
"$crexx_build/bin/rxlink" -o hello-linked hello.rxbin \
  c-runtime/library.rxbin c-runtime/llctarget.rxbin c-runtime/rxfnsc.rxbin \
  c-runtime/classlib.rxbin c-runtime/rxunicode-core.rxbin
"$crexx_build/bin/rxbvm" --no-autoload hello-linked.rxbin
```

Retain the prepared build's provider files and platform environment when using
this route. Dynamic programs additionally link the prepared
`$crexx_products/llcdynamic.rxbin`. Do not combine the standalone Unicode core
with a G aggregate that already contains the same procedures.

## Writing Classic source

Classic Rexx needs no type declarations for ordinary variables. An assignment
stores a value; expressions use its text or numeric meaning as needed. Instructions
and ordinary names are case-insensitive under the Classic casing policy, while
quoted data stays exact. A name that has no value normally evaluates to its
canonical name; enable NOVALUE when you want such reads trapped.

```rexx
options levelc
price = 12.50
quantity = 3
say 'Total:' price * quantity
message = 'It''s ready'              /* doubled quote inside a string */
say message
```

Physical line endings and semicolons separate clauses. A comma at a physical
line end continues the clause when more source follows. Nested `/* comments */`
are supported. Keywords are contextual: `nop = 'text'` is an assignment, whereas
bare `nop` does nothing. Put `options levelc` first; optional bare header words
can select STRICTC and supported comment policy. Later OPTIONS operands are
expressions whose effects execute, but the current runtime option processor
recognizes no option words.

### Variables, stems and assignment

A variable starts with a Unicode letter or `! ? _ @ # $`; later characters may
also be ASCII digits or dots. Combining marks and emoji are not identifier
letters, and names are not normalized. The reserved period variables are
`.MN`, `.RESULT`, `.RC`, `.RS` and `.SIGL`; other period constants are not general
variables.

Stems and compound names provide indexed data without an array declaration:

```rexx
options levelc
stock. = 0
item = 'PENCIL'
stock.item = 12
say stock.PENCIL                    /* 12 */
say stock.PAPER                     /* stem default: 0 */
drop stock.PENCIL
```

The tail `item` is substituted using its value. Assignment evaluates the right
side before resolving/writing the target, so calls on that side can affect a
compound tail. DROP removes a binding; assigning `''` instead stores an empty
value. `drop a b` processes names in order; `drop (names)` reads a variable's
space-separated name list once and drops those names. Stem/EXPOSE aliases use
the shared pool rules rather than copies of the variables.

### Values, operators and numeric precision

Concatenate with `||` for no inserted blank, or a source blank for one inserted
blank. Adjacent terms can also abut. A function name must be adjacent to its
opening parenthesis; `length('abc')` is a call.

| Strongest to weakest | Operators |
|---|---|
| Prefix | Unary `+`, `-`, `\` |
| Power | `**` |
| Multiplication | `*`, `/`, integer division `%`, remainder `//` |
| Addition | `+`, `-` |
| Concatenation | `\|\|`, blank, abuttal |
| Comparison | Normal `= \= < > <= >= <> ><`; strict `== \== << >> <<= >>=` and admitted negated aliases |
| Logical AND | `&` |
| Logical OR/XOR | `\|`, `&&` |

Power associates left and prefix binds more tightly than power. Logical
operators evaluate both operands. To avoid an invalid second operation, use
nested IF statements rather than relying on short-circuit evaluation.
Normal comparison uses Classic numeric/text rules; strict comparison compares
exact text. IF/WHEN/loop tests require logical `0` or `1`, not a numeric-looking
value such as `'1.0'`.

```rexx
options levelc
say '01' = '1'                     /* 1 */
say '01' == '1'                    /* 0 */
say 7 % 2                         /* 3 */
say 7 // 2                        /* 1 */
numeric digits 20
say 1 / 3
```

NUMERIC DIGITS, FUZZ and FORM change the current invocation. Defaults are
DIGITS 9, FUZZ 0 and SCIENTIFIC. DIGITS must exceed FUZZ and is limited to 999.
Use `numeric form engineering` or `numeric form value expression` for computed
form selection; bare FORM resets SCIENTIFIC. Internal routines inherit settings,
but changes in a child are restored on return. DIGITS(), FUZZ() and FORM()
report the current settings.

### Unicode and ordinal data

Source is UTF-8; ordinary values are Unicode text. LENGTH and ordinary
substring, search, reverse and PARSE positions count **codepoints**, not UTF-8
bytes or displayed graphemes. No implicit normalization occurs: `é` and `e`
followed by U+0301 remain different text, with lengths one and two respectively.

Classic C preserves compatibility across the Latin-1 ordinal range 0–255 in
both length and ordinary casing. Each ordinal is one character. For UPPER,
LOWER, default TRANSLATE, PARSE UPPER and canonical names, an input in
U+0000–U+00FF uses a simple case mapping only when the result stays in that
range. Otherwise it is unchanged. Ordinary UPPER therefore preserves `µ` and
`ÿ`; it does not expand `ß`. Inputs above U+00FF use simple Unicode mappings,
including mappings back into Latin-1: LOWER maps `Ÿ` to `ÿ`. Upper/lower are
therefore not guaranteed inverses, and the canonical names `ÿ` and `Ÿ` remain
distinct. This casing policy does not change codepoint length or positions.

That compatibility rule is specific to **Classic C**, including its B-written
runtime. It does not restrict ordinary Level B/G casing or the shared Unicode
API. Within C, explicit TOUPPERCASE/TOLOWERCASE and folding BIFs retain their
Unicode contracts: `touppercase('ÿ')` yields `Ÿ`, and `touppercase('ß')` yields
`SS`. Use explicit normalization or grapheme functions when those are the
operations intended; see the [Unicode reference](../../../../docs/books/crexx_library_reference/unicode.md).

Hex/binary literals map each decoded ordinal 0–255 to the same Unicode scalar.
Thus `'00'x` preserves NUL, `'80FF'x` denotes U+0080 and U+00FF, and `'C3A9'x`
differs from the single character `'é'`. Literal suffixes are adjacent and
case-insensitive. Byte-valued conversion/bitwise operations use this fixed
Latin-1 ordinal bridge and reject higher scalars with their Classic error;
they do not operate on the UTF-8 encoding of arbitrary text. Stream codecs are
explicit and do not change source decoding or ordinal meanings. See the
[Unicode and streams contract](../../../../compiler/docs/levelc_unicode_and_streams.md).

## Choosing and repeating work

IF chooses one instruction; group several instructions with DO/END. ELSE
belongs to the nearest unmatched IF. SELECT evaluates WHEN clauses in order
until one is true; OTHERWISE handles the remaining case.

```rexx
options levelc
score = 75
if score >= 50 then do
  say 'Pass'
  say 'Score:' score
end
else say 'Try again'

select
  when score >= 80 then say 'Distinction'
  when score >= 50 then say 'Qualified'
  otherwise say 'Below threshold'
end
```

DO supports a fixed count, a control variable, FOREVER, WHILE and UNTIL.
Grouping DO has no repetition. WHILE tests before entry; UNTIL tests after the
body. TO/BY/FOR expressions in a controlled header are evaluated once in written
order. The control variable itself can be changed by the body.

```rexx
options levelc
total = 0
do i = 1 to 5
  if i = 3 then iterate
  total = total + i
end
say total                           /* 12 */

do forever
  total = total - 1
  if total = 10 then leave
end
```

LEAVE exits the nearest repetitive DO; ITERATE continues it through its normal
UNTIL/advance logic. A name selects the nearest matching controlled DO, for
example `leave i`. Grouping DO and SELECT are not targets. By default invalid
static targets are compilation errors even in dead source. An initial
`options levelc strictc` instead reports invalid targets only if reached;
malformed syntax still fails compilation.

## Calling other routines

Use a function expression when you need a result, and CALL for a subroutine.
Place local routines after an EXIT or RETURN so main execution does not fall
through into them. A label marks the routine's entry; it is not a separate
function declaration or automatic scope boundary.

```rexx
options levelc
total = 40
call addtwo
say total                           /* 42 */
say greet('Ada')                    /* Hello, Ada */
exit

addtwo: procedure expose total
  total = total + 2
  return

greet: procedure
  parse arg name
  return 'Hello,' name
```

An internal call initially shares its caller's variables. PROCEDURE creates a
private pool only when executed; EXPOSE names the intended shared bindings.
Place PROCEDURE first after the entry label. Calls inherit numeric, ADDRESS,
TRACE and condition settings in a child activation, and restore caller state
on completion.

`parse arg` preserves argument text; standalone ARG uppercases it before parsing.
Omitted positions differ from present empty text:

```rexx
options levelc
call inspect , ''
exit
inspect: procedure
  say arg()                         /* 2 positions */
  say arg(1, 'O') arg(2, 'E')       /* 1 1 */
  return
```

A returned value from CALL sets RESULT/.RESULT; no result drops them. Present
empty text is a result. A function must supply a value: bare RETURN raises 45.1
in that function; absent EOF or another absent completion raises 44.1 at the
call. RETURN ends the current routine. EXIT ends its program root, including
internal calls above it. A separately invoked external Classic program has its
own root, so its EXIT completes that external call.

Unquoted calls resolve local labels, then built-ins, then external routines.
Quoting skips locals, but exact uppercase BIF spelling still works:
`'LENGTH'('abc')` calls the BIF even if you have a local LENGTH label.
Quoted lowercase `'length'` does not select that BIF. Arguments evaluate once
left to right. There is no dynamic CALL VALUE syntax.

### Precompiled providers and executable calls

You can explicitly select a precompiled Classic routine or an admitted scalar
B/G/RXPA procedure. The provider is built with its owning compiler first.
For a Classic provider, save this as `provider.rexx`:

```rexx
options levelc
parse arg value
return value || '!'
```

In the private writable installation used above, create its packet export:

```sh
"$crexx_bin/llrxc-c" -i "$crexx_bin" --routine sample.run \
  -o "$crexx_bin/provider.rxas" provider.rexx
"$crexx_bin/rxas" "$crexx_bin/provider.rxas"
```

A caller containing `say sample.run('hello')` selects it with:

```sh
"$crexx_bin/llrxc-c" -i "$crexx_bin" --provider provider sample.run \
  -o caller.rxas caller.rexx
```

Assemble the caller and include `provider.rxbin` alongside the normal runtime
inputs when linking. It returns `hello!`. Use a separate writable metadata
staging directory if your installation is read-only.

Ordinary scalar exports use `--provider`; RXPA exports use `--native` with the
owning provider's declaration. Present arguments convert to the declared scalar
types, omitted optional arguments use callee defaults, and a void result is
absent. Exposed scalar parameters get temporary cells whose updates are discarded.
Arrays, objects, true references, methods/factories and tasks require explicit
adapters. Importing a routine does not expose B/G globals to Classic variables.

`--executable runner /absolute/path/to/program` instead binds a routine alias to
an executable. `call runner 'one argument', ''` passes two direct argv elements.
The result is decimal exit-status text; stdout/stderr stay host streams. There
is no implicit shell parsing, CALL redirection syntax or executable PATH search.
Omitted actuals are refused, and NUL in OS paths/arguments is rejected before
launch. A selected provider/process failure never falls back to another routine.

## Parsing text and using queues

PARSE is a compact way to divide text without constructing a series of substring
calls. A single target receives the whole text. Several word targets split
initial words and leave the exact remainder in the last target. A dot consumes
a field without assigning it.

```rexx
options levelc
record = 'Ada:London:engineer'
parse var record name ':' city ':' job
say name city job
parse value 'one two three' with first . last
say first last                       /* one three */
parse value 'ABC123' with letters 4 digits
say letters digits                   /* ABC 123 */
```

A literal pattern such as `':'` splits at its next occurrence. `(variable)`
uses a live variable as the pattern. An integer gives a one-based absolute
position; `+n` and `-n` move relative to the previous match anchor. Parenthesized
variable positions are supported too. Targets are written in order, so later
patterns and compound target names see earlier completed assignments. An operand
needed to finish a section is read before that section is assigned.

Sources are ARG, PULL, SOURCE, LINEIN, VERSION, VALUE and VAR. VALUE takes an
expression followed by WITH; VAR takes a variable. SOURCE and VERSION provide
program/runtime descriptions. `parse upper ...` applies Classic UPPER to the
captured source, not to your patterns or original arguments. Comma templates
select successive ARG positions; for other sources later comma templates
receive empty text without reacquiring the source. PARSE EXTERNAL and PARSE
NUMERIC are outside this profile.

The selected queue supplies PULL before fallback input. PUSH adds at the front,
QUEUE at the tail, and QUEUED() counts without consuming:

```rexx
options levelc
queue 'second'
push 'first'
parse pull a
parse pull b
say a b                              /* first second */
```

Standalone PULL behaves as PARSE UPPER PULL. Bare PULL still consumes one item;
bare PUSH/QUEUE insert empty text. LINEIN reads the stream route rather than
consuming queued data. Queue selection and fallback input can be configured by
the embedding host.

## ADDRESS and embedding

ADDRESS sends commands to a named environment. A lasting selection changes the
active environment; a transient command uses its named environment for that
command. Bare ADDRESS swaps current and alternate environments. ADDRESS()
reports the current setting.

```rexx
options levelc
address SYSTEM 'echo Hello'
say 'Command status:' rc
```

This example needs a host with SYSTEM and an `echo` command. `SYSTEM`, `COMMAND`
and `CMD` use the platform command processor; `SHELL` uses the configured shell,
`PATH` direct executable dispatch, and `CREXX` the portable command environment.
A quoted expression used as an instruction is an implicit command in the active
environment. Unquoted expression heads can be accidental commands; the compiler
warns with `RXC-LC-IMPLICIT_ADDRESS`.

WITH connects INPUT/OUTPUT/ERROR to NORMAL, STREAM or STEM resources; output/error
can select REPLACE or APPEND. Resources are variable names, with a trailing dot
for a stem. For example, `address SYSTEM 'echo Hello' with output stem lines.`
places output in a counted stem under the shared command contract. A STREAM
resource reads its filename variable when the ADDRESS clause executes.
Lexical `:name`/`${name}` bindings use the environment's request/response protocol;
the environment defines their command meaning. They are not general string
interpolation syntax.

An embedding can supply invocation mode, exact arguments and omissions, queues,
streams, clock and environment services. Compile with `--host example.run` to
export a typed request/result entry instead of a command main. Its result keeps
absent versus present completion and owned conditions after invocation teardown.
Callback pool access is explicitly enabled and valid only during its synchronous
window. The [host facade guide](../../../../lib/rxfnsc/RexxClassicHost.md)
contains the B caller API/example; the [VM guide](../../../../docs/ai-context/RXVM_INTERPRETER.md)
covers native environment and embedding interfaces.

## INTERPRET and Rexx source calls

INTERPRET executes a string as Classic source in the current invocation:

```rexx
options levelc
x = 40
interpret 'x = x + 2'
say x                                /* 42 */
```

Save it as `dynamic.rexx`, compile/assemble normally and add dynamic support:

```sh
"$crexx_bin/llrxc-c" -i "$crexx_bin" -o dynamic.rxas dynamic.rexx
"$crexx_bin/rxas" dynamic.rxas
"$crexx_bin/rxlink" -o dynamic-linked dynamic.rxbin \
  "$crexx_bin/library.rxbin" "$crexx_bin/llctarget.rxbin" \
  "$crexx_bin/rxfnsc.rxbin" "$crexx_bin/classlib.rxbin" \
  "$crexx_bin/rxunicode-core.rxbin" "$crexx_bin/llcdynamic.rxbin"
"$crexx_bin/rxbvm" --no-autoload dynamic-linked.rxbin
```

The same C frontend validates the complete string before executing it through
semantic IR. No native source compiler or assembler is called at runtime.
Empty source succeeds. A group can contain nested INTERPRET and call local
routines, but cannot declare labels or leave a DO/SELECT open across its boundary.
It shares live arguments, variables, numeric settings, ADDRESS and condition
state. Effects of evaluating the source expression have already occurred when
a preparation error is reported.

RETURN in a group returns from its owning routine; SIGNAL transfers to an
eligible containing-program label; EXIT ends the applicable program root.
An outer loop is not a LEAVE/ITERATE target inside a group. A first generated
PROCEDURE can use the called invocation's still-valid entry eligibility; its
private pool survives group completion. Likewise a condition policy installed
by a group can remain active in later compiled code.

### Calling a source file

Save this as `child.rexx` in the execution working directory:

```rexx
options levelc
parse arg value
return value || '!'
```

A caller can then use:

```rexx
options levelc
say 'child.rexx'('hello')
```

Compile, assemble and link the caller with the same dynamic inputs. Running it
from that directory prints `hello!`. The child is read as a complete C program,
so labels are legal and it owns a fresh external root and variable pool.
Its EXIT ends the child, then the caller continues. A source call does not launch
an executable or return captured stdout.

### Where source calls look

After local/BIF and explicit binding resolution, a configured EXTERNALROUTINE
hook gets an opportunity. If unhandled, enabled source lookup proceeds:

1. Trusted caller resource directory, execution-context working directory, then
   configured roots, keeping the first occurrence of duplicate directories.
2. Within each directory, a bare stem tries `.rexx` then the exact extensionless
   name. Unquoted stems use Classic lowercase; quoted spelling/case stays exact.
3. An explicit extension means that exact filename. `.crexx` is not a default
   suffix. A path is exact: `./child` does not imply `./child.rexx`.
4. No implicit OS PATH, REXXPATH or compiler `-i` search. Every selected source
   file is validated as Classic C; a B/G header does not invoke another compiler.

A file-backed child supplies its directory for nested calls. A compiled initial
program's logical name or PARSE SOURCE text does not itself supply a trusted
source directory. Explicit relative paths use execution cwd. Names are not
trimmed; empty/NUL names have no filesystem candidates after the hook.

Only genuine absence advances to another candidate. A found but unreadable,
invalid or failing file stops the search. A handled hook with no result is also
final. Standalone dynamic scopes enable this policy with no extra roots;
`scope.configureSource(enabled, roots)` is the runtime configuration interface,
not a CLI switch. Embeddings must arrange their explicit policy.

## Conditions, TRACE and diagnostics

SIGNAL ON transfers to a handler in the same invocation. For example:

```rexx
options levelc
signal on syntax name bad
say 1 / 0
exit
bad:
  say 'Error near line' sigl
  say condition('D')
  exit
```

SIGNAL ON/OFF supports SYNTAX, ERROR, FAILURE, HALT, NOTREADY, NOVALUE and
LOSTDIGITS. A delivered SIGNAL disables its own policy before entering the
handler; rearm it explicitly if needed. Direct `signal label` and
`signal value expression` perform non-call transfers. Target names use canonical
Classic casing and exact text comparison, so numeric-looking `1.20` and `1.2`
remain different labels.

CALL ON/OFF supports ERROR, FAILURE, HALT and NOTREADY. These handlers run at a
clause checkpoint and normally return to the interrupted program. They receive
zero arguments, and their return values are ignored. The runtime suppresses
repeated delivery of the same active delayed event and buffers HALT under its
shared rules. SIGNAL transfers rather than resuming. Unhandled events are
terminal unless an explicit host completion boundary captures them.

Use `trace i` for intermediate records, `trace r` for results and `trace o` to
turn tracing off. Bare TRACE resets Normal. `trace value expression` computes
an option; TRACE() queries state or returns the old setting while changing it.
Values retain Unicode and escape controls such as NUL in output.

TRACE has deliberate limits: no interactive prompt, no enacted nonzero
skip/pause/suppression counts, no SCAN and no complete expression-event coverage.
Labels mode is quiet; command N/E/F classification is coarse. File-transition
markers and some multiline/BIF/literal observations differ between implementations.
Use [the practical profile](levelc-conformance.md#practical-trace-target) when
comparing transcripts; wrong values or changed execution remain defects.

Compile errors carry a logical filename, line/scalar column and source context.
Static malformed source fails even in untaken arms. Reached numeric/missing-call
errors stay runtime events. INTERPRET preparation failures report Classic SYNTAX
with generated-source detail. SOURCELINE reads retained compiled lines, not the
current disk file; source-mapped input has no original inventory unless its map
actually supplies the text needed for an observation.

## Built-in function reference

The compiler supplies 95 names: the 70-name Classic catalogue, LOWER/UPPER and
23 Unicode extensions. No source import is needed. In the calling forms below,
square brackets mean optional arguments; do not type the brackets. A comma hole
omits a position, while `''` supplies empty text. Arguments evaluate left to
right, and each function owns its defaults and argument errors.

Positions are normally one-based codepoints and unsuccessful searches return
zero. Word functions use the runtime's configured blanks. Ordinary padding is
one character, usually a blank. The exceptions—INSERT's insertion offset,
explicit byte ordinals and grapheme positions—are stated where used.

### Measuring, slicing and editing text

| Calling form | Purpose and example |
|---|---|
| `LENGTH(text)` | Codepoint count: `length('α漢🙂')` is 3. |
| `LEFT(text,n[,pad])`, `RIGHT(text,n[,pad])` | Take/pad to exactly n characters: `left('ab',4,'.')` is `ab..`. |
| `CENTER(text,n[,pad])`, `CENTRE(text,n[,pad])` | Center by trimming or padding; the names are aliases. |
| `SUBSTR(text,start[,length[,pad]])` | Slice/pad from start. `substr('abcdef',2,3)` is `bcd`; omitted length selects the suffix, zero selects empty. |
| `DELSTR(text,start[,length])` | Remove a character range; omitted length removes the suffix. |
| `INSERT(new,target[,after[,length[,pad]]])` | Insert after a count of characters (default 0); `insert('X','ab',1)` is `aXb`. Length selects/pads inserted text. |
| `OVERLAY(new,target[,start[,length[,pad]]])` | Replace from a one-based start (default 1), extending/padding as needed. |
| `STRIP(text[,option[,char]])` | Strip leading L, trailing T or both B (default), using a blank by default. |
| `COPIES(text,n)` | Repeat text n times; zero gives empty text. |
| `REVERSE(text)` | Reverse codepoints; use GRAPHEMEREVERSE for visible clusters. |
| `CHANGESTR(needle,text,replacement)` | Replace matching occurrences: `changestr('a','banana','o')` is `bonono`. |
| `LOWER(text)`, `UPPER(text)` | Apply ordinary Classic casing to the whole text. |
| `TRANSLATE(text[,outtable[,intable[,pad]]])` | Map characters using parallel tables; with no tables, apply ordinary uppercase. `translate('abc','XY','ab')` is `XYc`. |

### Searching and comparing text

| Calling form | Purpose and example |
|---|---|
| `POS(needle,text[,start])` | First match at/after start; `pos('na','banana')` is 3. |
| `LASTPOS(needle,text[,start])` | Last match searching backward from the specified position, or end by default. |
| `COUNTSTR(needle,text)` | Count nonoverlapping occurrences; `countstr('an','banana')` is 2. |
| `COMPARE(left,right[,pad])` | Position of first difference, padding the shorter text; 0 means equal. |
| `ABBREV(full,short[,minimum])` | Test whether short is a prefix of full of sufficient length. `abbrev('PRINT','PRI',2)` is 1. |
| `VERIFY(text,reference[,option[,start]])` | Find the first character outside reference (N, default), or inside it (M). `verify('12x','0123456789')` is 3. |

### Working with words

| Calling form | Purpose and example |
|---|---|
| `WORDS(text)` | Count words; `words('one two three')` is 3. |
| `WORD(text,n)` | Select the nth word, or empty if missing. |
| `WORDINDEX(text,n)` | Codepoint position of the nth word, or 0. |
| `WORDLENGTH(text,n)` | Length of the nth word, or 0. |
| `SUBWORD(text,start[,count])` | Select words; omitted count takes the rest. |
| `DELWORD(text,start[,count])` | Remove words; omitted count removes the rest. |
| `WORDPOS(phrase,text[,start])` | Find a sequence of words, returning its first word number. |
| `SPACE(text[,count[,pad]])` | Remove outer blanks and put count separators between words (default 1). `space(' a  b ',1,'-')` is `a-b`. |

### Numbers and formatting

| Calling form | Purpose and example |
|---|---|
| `ABS(number)` | Absolute value. |
| `SIGN(number)` | -1, 0 or 1 according to sign. |
| `MAX(number[,number...])`, `MIN(number[,number...])` | Numeric maximum/minimum of the supplied values. |
| `TRUNC(number[,places])` | Truncate to places decimal digits (default 0); `trunc(12.345,2)` is `12.34`. |
| `FORMAT(number[,before[,after[,expp[,expt]]]])` | Format integer/fraction widths and exponent width/trigger using current numeric settings. |
| `DATATYPE(value[,type])` | Without type, NUM or CHAR; with a type, 1/0. Options include A alphanumeric, B binary, L lowercase, M mixed-case alphabetic, N numeric, S symbol, U uppercase, W whole and X hex. |
| `DIGITS()`, `FUZZ()`, `FORM()` | Current numeric settings; change them with NUMERIC. |
| `RANDOM([max])` or `RANDOM([min[,max[,seed]]])` | Integer in the inclusive range; default 0–999. One argument supplies the maximum; a third argument sets the configured seed. |

### Ordinal, hexadecimal and bit operations

These functions treat U+0000–U+00FF as ordinals 0–255. They do not reinterpret
UTF-8 storage. Byte-only inputs above that range are rejected. Explicit widths
control truncation, padding and signed interpretation as defined by each function.

| Calling form | Purpose and example |
|---|---|
| `C2X(text)` | Ordinals to hex: `c2x('é')` is `E9`. |
| `X2C(hex)` | Hex to ordinal text: `x2c('00FF')` contains NUL and U+00FF. |
| `C2D(text[,length])` | Ordinal bytes to decimal; an explicit length selects signed-width interpretation. |
| `D2C(number[,length])` | Whole decimal to ordinal bytes, optionally fixed width. |
| `X2D(hex[,length])`, `D2X(number[,length])` | Hex/decimal conversion; width is in hex digits. |
| `B2X(binarydigits)`, `X2B(hex)` | Binary digit strings to/from hexadecimal. |
| `BITAND(left[,right[,pad]])`, `BITOR(left[,right[,pad]])`, `BITXOR(left[,right[,pad]])` | Bitwise ordinal-string operations, with optional extension padding. |
| `XRANGE([first[,last]])` | Inclusive ordinal character range; omitted endpoints select the full 0–255 range. |

For UTF-8 bytes use ENCODE explicitly: `c2x(encode('é'))` is `C3A9`, and
`decode(x2c('C3A9'))` is `é`.

### Explicit Unicode functions

These functions explicitly request Unicode operations beyond ordinary Classic
casing and codepoint indexing. Optional arguments use the same notation as above.

| Calling form | Result |
|---|---|
| `UNICODEVERSION()` | Pinned Unicode version reported by the shared core. |
| `TONFD(text)`, `TONFC(text)`, `TONFKD(text)`, `TONFKC(text)` | Text in the selected normalization form. |
| `ISNFD(text)`, `ISNFC(text)`, `ISNFKD(text)`, `ISNFKC(text)` | Whether text already has that normalization form. |
| `TOUPPERCASE(text)`, `TOLOWERCASE(text)` | Full Unicode case conversion, including supported expansions and context rules. |
| `TOCASEFOLD(text)`, `TOSIMPLECASEFOLD(text)`, `TOTURKICCASEFOLD(text)`, `TOTURKICSIMPLECASEFOLD(text)` | The explicitly selected default/simple/Turkic folding form for caseless comparison. |
| `GRAPHEMECOUNT(text)`, `GRAPHEMEREVERSE(text)` | Count or reverse extended grapheme clusters, preserving scalar order within each cluster. |
| `GRAPHEMESUBSTR(text,start[,length[,pad]])` | Slice using one-based grapheme positions; omitted length selects the suffix, explicit zero selects empty, and padding is one grapheme. |
| `GRAPHEMEPOS(needle,text[,start])` | Find exact text with both ends on grapheme boundaries; default start is 1, and no match returns 0. |
| `ENCODE(text[,encoding[,replacement]])` | Encoded bytes represented as Classic ordinal characters U+0000–U+00FF. |
| `DECODE(ordinals[,encoding[,replacement]])` | Unicode text decoded from those ordinal characters. |
| `ISDECODABLE(ordinals[,encoding])`, `ISENCODINGSUPPORTED(encoding)` | Test data validity for a codec or whether a codec is supported. |

ENCODE, DECODE and ISDECODABLE default to UTF-8 when encoding is omitted;
an explicit empty encoding is invalid. Omitting replacement requests strict
conversion. ENCODE replacement is a nonempty sequence of target-encoded byte
ordinals; DECODE replacement is nonempty text. No BOM is inserted or consumed
automatically. Strict conversion failures raise Classic SYNTAX 23.1;
unsupported encoding names and invalid options are argument errors.
ISDECODABLE returns false for malformed data but still rejects an unsupported
encoding; ISENCODINGSUPPORTED returns false for an unsupported name.

For example, `c2x(encode('é'))` yields `C3A9`, whereas `c2x('é')` yields `E9`:
the first explicitly encodes UTF-8 and the second uses the fixed ordinal bridge.
`decode(x2c('C3A9'))` yields `é`. The
[Classic Unicode/stream contract](../../../../compiler/docs/levelc_unicode_and_streams.md)
and [Unicode library reference](../../../../docs/books/crexx_library_reference/unicode.md)
provide codec spellings, normalization, folding and grapheme details. B/G use
typed `.binary` codec values; the C adapters deliberately use Classic ordinal
strings instead.

### Arguments, variables, queues and execution state

| Calling form | Purpose |
|---|---|
| `ARG([n[,option]])` | No n: positional argument count. With n: its value. E tests existence, O omission; present empty is distinct from omitted. |
| `VALUE(name[,newvalue[,pool]])` | Read by name; when newvalue is present, assign and return the old value. `value('x',,pool)` selects an external configured pool without assignment. |
| `SYMBOL(name)` | Classify a symbol as BAD, LIT or VAR under current pool rules. |
| `QUEUED()` | Selected queue depth without consumption. |
| `ADDRESS([option])` | Query current environment or its supported connection information; options E/I/N/O follow the shared command interface. |
| `CONDITION([option])` | Trapped condition data: C name, D description, E extra, I instruction (default) or S state. |
| `ERRORTEXT(number[,option])` | Classic error text; the optional S/N form selects catalog presentation. |
| `SOURCELINE([n])` | Retained line count without n, or the nth source line. It does not reread the source file. |
| `TRACE([option])` | Return the previous setting, optionally installing a new one. Practical limitations apply. |

### Streams, clocks and dates

Stream functions use host-configured streams and codecs. Omitted names select
the corresponding default stream. Logical character positions are codepoints;
raw-byte work uses the explicit codec/provider facilities. EOF, unavailable
operations and NOTREADY handling depend on the selected stream and host.
The [stream implementation contract](../../../../compiler/docs/levelc_unicode_and_streams.md)
explains encodings, positions and close behavior.

| Calling form | Purpose |
|---|---|
| `CHARIN([name[,start[,length]]])` | Read characters; default length 1. A start requests a logical position. |
| `CHAROUT([name[,text[,start]]])` | Write characters; returns the unwritten count. Omitted text requests the stream's close/position behavior. |
| `CHARS([name[,option]])` | Remaining character count or availability under C/N/I options. |
| `LINEIN([name[,line[,count]]])` | Read a line without its terminator; count is 0 or 1 (default 1). |
| `LINEOUT([name[,text[,line]]])` | Write a line; returns 0 on success or 1 on failure. Omitted text requests close, or position-only behavior when line is supplied. |
| `LINES([name[,option]])` | Remaining line count or availability under C/N/I options. |
| `QUALIFY([name])` | Host-qualified stream name. |
| `STREAM(name[,option[,command]])` | Query state S (default), description D, or issue command C, such as `stream(name,'C','CLOSE')`. |
| `DATE([option[,date[,inoption]]])` | Current date or conversion of a supplied date. N is normal, S sortable, E European, U US, O ordered, B base days, D day of year, M month and W weekday; input options are B/D/E/N/O/S/U. |
| `TIME([option[,time[,inoption]]])` | Current time or conversion. N is normal; C civil, H/M/S totals, L long, O offset, E elapsed and R reset. Conversion input options are C/H/L/M/N/S; E/R/O are not conversion outputs. |

DATE/TIME queries within one authored clause share a lazy sample; later evaluated
clauses refresh it. Elapsed/reset state retains the current UTC-based contract.
It is not a promise of a monotonic duration clock. Host clock/random and stream
facilities can be replaced or restricted through configuration.

The [binding inventory](levelc-conformance.md#runtime-binding-inventory)
links the complete name set to its owning runtime entries. These calling forms
are a programmer's reference; exact edge-case argument/error rules remain with
those implementations and the conformance profile.

## Compiler options

```text
llrxc-c [-n] -i runtime-images -o output.rxas
        [--provider artifact-stem qualified-callable]
        [--native artifact-stem qualified-callable]
        [--executable alias path]
        [--routine qualified-export | --host qualified-export]
        source-file
```

| Option | Meaning |
|---|---|
| `-i directory` | Required target runtime-image and explicit provider metadata directory. |
| `-o path` | Required completed RXAS output path. |
| `-n` | Disable optional semantic IR optimization. Admission and mandatory construction cleanup remain active. |
| `--provider artifact callable` | Select an existing RXBIN Classic packet export or admitted ordinary typed scalar procedure. The artifact stem is resolved under `-i`. |
| `--native artifact callable` | Select an RXPA callable from its owning native provider declaration and identity. |
| `--executable alias path` | Bind a routine alias to an explicit executable, using direct argv and shared process services. |
| `--routine export` | Generate a signed Classic activation-packet provider instead of a command main. |
| `--host export` | Generate an ordinary typed host request/result export instead of a command main. |

Binding options are repeatable. They share one external alias namespace;
qualified names can disambiguate providers. `--routine` and `--host` are mutually
exclusive. The CLI exposes no source-root, dynamic-runtime-budget or compiler
work-budget switches; those belong to the corresponding service/runtime APIs.

Compiler and assembler optimization are independent. To inspect the compiler's
unoptimized result, use both `llrxc-c -n ...` and `rxas -n ...`. Native B seed
`rxc -n` is a third, build-time choice. An optimized compiler host does not
force optimization of every source request.

The command returns 0 after successful publication, 2 for command-line parsing
errors, and 1 for input, request validation, compilation or publication failures.
Warnings go once to stderr. Compilation and output errors leave an existing
destination intact. The command writes and closes the complete output in a
claimed sibling staging directory before publishing it. An existing staging
claim is an error, so avoid simultaneous writers to one destination.

## Embedding the compiler service

A B host can import `llccompiler` and submit exact UTF-8 `.binary` source with
an explicit artifact directory. The service does no command I/O, file publication,
process exit or native source compilation. Its constructor is:

```text
request(source, artifacts='', filename='<source>',
        budget=1000000, nesting=512, bytes=67108864,
        registers=65536, optimise=1,
        providers?, routine_export='', host_export='')
```

The host supplies `source` and `artifacts`; this fragment shows result handling:

```rexx
options levelb
import llccompiler
import llir

request = .llccompiler..request(source, artifacts, 'examples/demo.rexx')
if request.status() <> 0 then say request.diagnostic()
else do
  output = request.output()
  say output.text()
end
```

Link the B host with `llccompiler.rxbin`, `rxfnsl.rxbin`, `library.rxbin` and
selected provider requirements. Compiler service dependencies are distinct from
the runtime inputs needed by its generated application.

`status()`, `error()`, `offset()`, `diagnostic()` and `output()` complete the
request lazily at most once. `graph()` prepares frontend IR; `emissiongraph()`
after success returns the selected graph, possibly transformed. `warnings()`
prepares the frontend and owns records even if later work fails. Successful
output remains owned independently; a failure exposes no partial executable.
Optional provider definitions and routine/host exports follow the same checked
contracts as the CLI. Register budgets below 16 are invalid. Limits protect
specified phases; they are not a single process-memory or CPU cap.

## Diagnostics and troubleshooting

| Symptom | Check |
|---|---|
| `llrxc-c` or a research image is missing | Enable research compilers, build both stages, then install base and ResearchCompilers. Installation does not prepare outputs. |
| Metadata/import error | `-i` must contain the intended complete runtime metadata and selected providers; verify stems, signatures and provider identities. |
| Link error or missing procedure | Include selected provider bodies and correct static/dynamic images. Compile-time selection is not linking. |
| Duplicate Unicode procedures | Use one Unicode aggregate: standalone B core for C, or a deliberate aggregate that contains it. |
| Source call reports 43.1 | Check exact spelling/case, execution cwd, extension and configured roots. `-i` and PATH do not supply source roots. |
| Source lookup stops at a bad candidate | This is deliberate: only genuine absence permits the next candidate. Fix the found source or its access problem. |
| Source text is unavailable | Keep link metadata and distinguish compiled lines from mapped/stripped text. A disk file is not the retained inventory. |
| Unexpected host command | Read `RXC-LC-IMPLICIT_ADDRESS`; write an explicit assignment, function/CALL or ADDRESS as intended. |
| TRACE differs from another processor | Compare supported events and actual behavior against the practical profile. |
| A resource limit is reached | Read the failing phase. Compiler and dynamic limits are independent of shell timeouts. |

## Conformance and compatibility

This compiler implements the agreed cREXX Classic profile, including all
instruction families described here and all 95 built-in names. That does not
claim complete compatibility with every Rexx processor or formal conformance
for every argument/platform combination. Independent expected behavior governs
qualification; a native compiler defect is not behavior to copy.

The main porting considerations are:

- Ordinary strings use Unicode codepoints; Latin-1 ordinal and ordinary casing
  rules preserve the stated 0–255 compatibility. Full Unicode casing,
  normalization, graphemes and codecs are explicit functions.
- ARG/PULL uppercase; PARSE ARG/PULL preserve text unless UPPER is requested.
  PARSE EXTERNAL/NUMERIC are excluded.
- Logical operands are eager; strict comparisons preserve exact text. SIGNAL
  uses the agreed canonical-name lookup, including quoted/computed targets.
- TRACE is practical observation with partial coverage, not an interactive
  debugger. Its documented presentation limits are intentional.
- Calls use explicit scalar/provider/process interfaces. Host capabilities and
  source search are configured; no universal executable or ambient PATH fallback
  is implied.
- INTERPRET accepts complete label-free groups in live caller state. A source
  program instead owns a fresh external root. Both need the dynamic image.

Dynamic execution defaults to a 64 MiB logical invocation-tree budget, 16 MiB
plan cache and per-source limit, 64 nested groups, 64 IR frames per session,
100,000 live cells and 1,000,000 session dispatches. These are accounting bounds,
not RSS or host-code timeout guarantees; the
[architecture](levelc-architecture.md#implementation-interfaces-and-limits)
explains their ownership.

The [conformance profile](levelc-conformance.md) gives detailed differences and
known defects; [PLAN.md](../PLAN.md#c-first-delivery-milestone) defines the selected
C research closure. Broader reference conformance, installed/relocated delivery,
declared platforms, sanitizers and production replacement remain unqualified
future work, not additional tests commissioned by this guide. Real Windows,
Linux leak checking and constrained CMS/TSO/31-bit behavior cannot be inferred
from local component checks. The research compiler remains explicitly selected;
release, default switching and publication require separate authority.
