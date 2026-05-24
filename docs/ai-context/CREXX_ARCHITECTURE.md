# cREXX Architecture

`crexx` is a custom REXX-to-bytecode toolchain that translates Classic REXX semantics into an optimized bytecode format executed by a specialized VM. The process happens through four main binaries:
1. `rxc` - The Compiler
2. `rxas` - The Assembler
3. `rxlink` - The Linker
4. `rxvm` - The Virtual Machine (Interpreter)

## The Compilation Pipeline

The pipeline of transforming REXX source code into executable bytecode is structured as follows:

1. **re2c (Lexical Analyzer)**
   - Used to generate the scanner/lexer from `.re` rules (e.g., `compiler/rxcpbscn.re` and `assembler/rxasscan.re`).
   - Converts the raw REXX source text into a stream of discrete tokens (`Token` structs).

2. **Lemon (Parser Generator)**
   - The token stream is parsed using a grammar defined in `Lemon` (e.g., `compiler/rxcpbgmr.y` and `compiler/rxcpopgr.y`).
   - Lemon applies the grammar rules to recognize the syntactic structures of the REXX language and translates them into an Abstract Syntax Tree (AST).
   - `DO ... END` is overloaded: statement-leading `DO` remains the normal grouped/loop form, while expression-position `DO ... END` becomes `BLOCK_EXPR`. The grammar resolves the command-start ambiguity by routing top-level command expressions through a restricted `command_expression` spine while leaving the general expression grammar free to accept block expressions.

3. **AST (Abstract Syntax Tree) & Compiler Exits**
   - The primary data structure bridging the parser and the code emitter.
   - Built using a hierarchical structure of `ASTNode` C structs that capture operations, scopes, typing, and tree associations.
   - String comparison operators (`==`, `>>`, `<<`, `>>=`, `<<=`) are carried as a distinct `OP_COMPARE_S_*` family. During type validation both operands are retargeted to `TP_STRING`, but the optimiser still preserves intrinsic numeric constant types long enough to stringify from value rather than source spelling. That keeps folded behaviour aligned with runtime cases like `01 == 1`.
   - Inline cloning must preserve the callee scope's numeric context when it builds replacement scopes. Without that, folded float/decimal string comparisons can silently drift from runtime behaviour under local `NUMERIC DIGITS` / `FORM` / `CASE` settings.
   - Expression-level control flow is supported through `BLOCK_EXPR` (`DO ... END` used as an expression) and `LEAVE_WITH` (`LEAVE WITH expr`). The `association` pointer links each `LEAVE_WITH` back to its owning `BLOCK_EXPR`, similar to how loop `LEAVE` / `ITERATE` link to `DO`.
   - Contains the **Exit Bridge Framework** (`rxcp_exit.c`), which intercepts unrecognized `IMPLICIT_CMD` nodes, invokes user-provided `rxplugin` macros to generate replacement source code, parses the interpolated strings (preserving literal quotes), and surgically grafts the resulting AST back into the main tree without violating return-type constraints.
   - **Implicit Main Argument Bridge**: when the compiler synthesizes the file-level `main()` wrapper, that procedure is marked `is_implicit_main`. Later typing and emission use that marker to interpret classic `arg()` / `arg[]` / `arg[n]` access against the hidden command-line `.string[]` that the VM already passes to `main`. Ordinary procedures still use normal vararg semantics, and explicit zero-argument `main()` does not gain accidental source-level visibility of the hidden VM argv payload.
   - **Exit Fragment Scope Lifecycle**: replacement fragments from exits are parsed and structurally normalized before grafting, but any new lexical block scopes created by structured replacements are finalized later during symbol structuring/build. Nested `DO` / `IF` / `INSTRUCTIONS` emitted by exits are therefore a supported shape, and debug validation is staged after that scope rebuild so the validator sees the stabilized tree rather than the transient pre-scope fragment form.
   - **Expose Mechanics**: Implements automatic scope resolution that allows `namespace ... expose` global variables to implicitly bind into local `PROCEDURE` scopes. Procedure-level `name: procedure [= .type] expose var ...` remains the local form for selected private module state shared by specific procedures.
   - **Automatic Register Allocation**: Within `rxcp_val_sym.c` (Step 3 - Pass 3), the compiler walks the AST (`build_symbols_walker`) to identify explicit `NODE_REGISTER` allocations via the `with register.N[.view]` clause on class attributes. It then automatically maps any remaining unmapped attributes of a class to unused VM registers (`r1`, `r2`, etc.) by synthesizing implicit `NODE_REGISTER` AST nodes. For normal classes, prefer this implicit allocation and keep callers on factories/methods; explicit `with register...` mappings should be reserved for genuine physical interop where a fixed layout is required.

4. **Emitter (IR -> Assembly)**
   - AST walkers (e.g., `rxcp_ast_walk.c`, `rxcp_emit_*.c`) traverse the tree.
   - Outputs intermediate string fragments representing the `rxas` Assembly instructions.
   - The optimiser performs opportunity-based AST inlining before emission. A
     callable may be structurally available for inlining, but each call site is
     still validated against the supported rewrite shapes. The release slice
     uses a 300 AST-node body cutoff as a profitability and metadata-size
     policy, not as a semantic safety boundary. Unsupported semantic gates
     still fail closed, including procedure-level `expose`, assembler aliasing,
     dynamic-index varargs, receiver/copyback shapes without proof, and
     interface/member dispatch that cannot be proved monomorphic.
   - Inlining is implemented as AST tree surgery, not textual substitution.
     The cloned body must preserve callee-local scopes, remap symbols, bind
     arguments exactly as a real call would, preserve source/debug metadata,
     and leave the downstream emitter with an ordinary validated tree. This is
     why new inline cases are opened by specific parent/operand shape instead
     of by globally deciding that a callable is "always inlineable".
   - Cross-file inlining uses compiler-owned `META_INLINE` payloads alongside
     normal callable metadata. Libraries preserve this metadata for downstream
     `rxc` optimisation; final linked images strip it by default.

5. **Assembler (`rxas`)**
   - Parses the generated `rxas` Assembly instructions.
   - Translates human-readable IR assembly into packed binary format (`rxbin` bytecode).
   - Generates the final executable bytecode file.

6. **Linker (`rxlink`, optional)**
   - Combines one or more `.rxbin` modules into a single linked image with one shared constant-pool record and one shared-backed module record per selected module.
   - Resolves imports and interface-provider relationships up front, while preserving the module boundaries needed by the VM loader.
   - Can strip source/file metadata (`META_SRC` and `META_FILE`) for smaller deployable artifacts without removing runtime contract metadata.

7. **Interpreter (`rxvm`)**
   - `rxvm` reads and executes the `rxbin` bytecode.
   - Modules are loaded via `rxldmod`.
   - The execution loop happens inside the `rxvm_run` function (e.g., in `rxvmmain.c` / `rxvmintp.c`).

## Text, UTF-8, and Binary Data

In normal UTF builds, `.string` register data is stored as UTF-8 bytes while
the VM exposes character operations as codepoint operations. A VM `value`
tracks `string_length` as the byte length and, in UTF builds, also tracks
`string_chars` plus a byte/codepoint cursor pair (`string_pos` and
`string_char_pos`). Instructions such as `strlen`, `strchar`,
`setstrpos`/`getstrpos`, `substr`, `strpos`, `appendchar`, `fndblnk`, and
`fndnblnk` operate in codepoint space and use helpers such as
`string_set_byte_pos()`, `string_slice_from_cursor()`, and
`string_concat_char()` to walk or synthesize the underlying UTF-8 bytes. Some
scan paths have ASCII fast paths when byte length and codepoint count match.
These string instructions assume valid UTF-8 in the register payload. NUTF8
builds collapse this model back to byte positions and byte lengths.

Hex and binary suffixed source strings currently still enter the compiler as
`STRING` AST nodes. `ast_fstr()` validates the hex or binary digit syntax,
turns the decoded bytes into escaped RXAS string text, and `rxas` later
unescapes that text into a `STRING_CONST`. The assembler records
`string_chars` with `utf8nlen()`, which counts codepoints but does not validate
UTF-8 well-formedness. Therefore a literal such as `'FFFFFF'x` can enter the
string constant path as non-UTF-8 bytes and later reach character opcodes that
are written for UTF-8 strings. This is the current root cause behind issue 466.

`.binary` is present in the Level B surface and compiler metadata as
`TP_BINARY`. The VM `value` has a separate `binary_value`, `binary_length`, and
`binary_buffer_length` slot. Copies and moves preserve that slot, socket and
file byte operations use it (`socksendb`, `sockrecvb`, `freadb`, `fwriteb`),
and native payloads reuse it with `rxvm_native_payload_ops`. The binary path is
much thinner than the string path: there is no general binary append or cursor
helper equivalent to the string helpers, some construction and literal cases
still route through ordinary string loading, `GETBYTE` is still a stub, and
current read/receive paths tend to allocate or resize to exact byte counts
rather than sharing the string buffer growth machinery.

At the RXAS level, `BINARY_CONST` and `OP_BINARY` support exist and `0x...`
operands can be converted into constant-pool binary records, but the current
generated opcode formats do not expose a general binary-immediate instruction
family. Most character and string opcodes take string operands and assume
valid UTF-8 in UTF builds.

Level C text and binary behavior should be treated as design space, not as
settled current compiler behavior. Classic REXX is byte-oriented and commonly
stores binary data in the same text values used for strings, while current
Level B separates the intended surfaces as `.string` and `.binary`. Any Level C
compatibility mode therefore has to choose where Classic byte-text semantics
map: to UTF-8 `.string` semantics, to `.binary`, or to an explicit option such
as `bytetext`. Classic REXX BIFs will need to be audited against that decision.
Level G and library work have a separate Unicode extension path for grapheme,
word, and sentence boundaries, normalization, and case operations through the
Unicode plugin hooks; those features sit above the core codepoint-level VM
string contract.

### Locked Direction

The architecture direction is:

- `.string` means valid UTF-8 text in normal UTF builds.
- `.binary` means arbitrary bytes.
- Level B keeps those surfaces strict and typed; invalid byte streams should not
  silently become `.string` values.
- Level C may provide Classic REXX byte-text compatibility through an explicit
  compatibility mode such as `bytetext`, but that mode must not weaken the
  Level B/G `.string` contract.
- Level G should build richer Unicode services through the existing Unicode
  plugin hooks. `utf8proc` is the preferred first implementation candidate for
  normalization, case folding, Unicode property checks, and grapheme/word/
  sentence segmentation because it is a small C library under MIT expat plus
  Unicode data license terms.

Trust boundaries for `.string` validation are compiler/assembler string
constants, native/RXVML string setters, text file and socket reads, ADDRESS
callbacks, and any explicit byte-to-text conversion API. Internal operations
that preserve validity, such as copying, concatenating two already-valid
strings, slicing on codepoint boundaries, and appending a valid Unicode scalar,
should propagate cached validity/count state rather than rescanning.

The VM register/value status word is a `uint32_t` field partitioned in
`binutils/include/rxflags.h` instead of adding a second flag field:

- `0x000000FF`: VM-private, externally readable but not writable through RXAS
  flag instructions. This band is reserved first for UTF-8 validity/count and
  Unicode normalization-form cache bits.
- `0x0000FF00`: compiler call ABI flags. The current bits are `REGTP_VAL`
  (`0x00000100`) and `REGTP_NOTSYM` (`0x00000200`).
- `0x00FF0000`: stable library/runtime ABI flags.
- `0x7F000000`: user/experimental flags.
- `0x80000000`: reserved to avoid signed integer ambiguity.

`SETTP`, `SETORTP`, and `LOADSETTP` mask external writes so VM-private bits are
preserved or cleared only by VM internals. `GETTP`, `GETANDTP`, and explicit
`BRTPANDT` masks may observe readable VM-private bits; unmasked `BRTPT` only
tests public/external flag bands so VM cache bits do not change old branch
semantics.

RXAS/RXBIN integer operands remain `rxinteger`; status instructions cast masks
to the 32-bit flag word before applying the partition.

The implementation roadmap is:

1. Add a core validate-and-count helper for bounded UTF-8 byte spans.
2. Enforce valid UTF-8 for assembler `STRING_CONST` creation and compiler
   string literal lowering; route arbitrary byte literals to `.binary`.
3. Partition the existing register status word and reserve VM-private UTF/
   normalization cache bits rather than adding a second flag field.
4. Improve `.binary` with buffer-growth helpers, binary literal/load support,
   byte indexing/update instructions, and binary slice/concat operations.
5. Add runtime string-boundary signal plumbing for native/RXVML setters and
   text I/O paths that can currently receive arbitrary bytes.
6. Implement the Level G Unicode plugin with `utf8proc`.
7. Define Level C byte-text compatibility and migration rules explicitly,
   including how Classic REXX BIFs behave in byte-text versus UTF modes.

## Compiler Import Discovery

`rxc` does not treat every import location as both source and binary
space anymore. Import discovery is now split into two root classes:

- source roots for `.crexx`, `.crx`, `.rexx`, and the arbitrary extension used
  by the initial source file, if any
- binary roots for `.rxbin`, optional `.rxas`, and `.rxplugin`

The primary source root is the directory containing the source file
being compiled. Additional source roots come from `-s`. Binary roots
come from any `-i` paths and the compiler executable directory.
Repeated `-i` and `-s` options are accumulated in order. Search order
keeps project source files ahead of deployed binary artifacts.

An extensionless initial `rxc` input falls back to `.crexx`. Headerless
`.crexx`, `.crx`, and arbitrary-extension sources default to Level G;
headerless `.rexx` defaults to Level C. `.rxpp` is reserved for the
preprocessor and is not scanned as an import source extension.

For source discovery the compiler now performs a lightweight header scan
before any full parse. That scan reads the leading `options`,
`namespace`, and `import` clauses so namespace-invisible files can be
rejected before `rexbpars()` and `rxcp_val()` are invoked. Full source
parsing is still used once a source file is actually selected as an
import candidate.

Within a binary root, same-stem artifacts are collapsed to the freshest
candidate. If timestamps tie, `.rxbin` is preferred over `.rxas`.

## Level B Classes and Interfaces

Level B interface support is now implemented across the compiler, assembler
metadata path, and VM.

### Compiler model

The source surface includes:

- `interface`
- `class implements .iface ...`
- interface methods with or without bodies
- interface default `*` factories and named factories
- class-side same-named factory implementations
- optional class-side same-named `match`
- checked casts with `expr as .type`
- boolean type tests with `expr is .type`
- concrete type introspection with `typeof(expr)`
- namespace-qualified contracts such as `.pkg..thing()`

Interface methods with bodies are emitted as final/default methods. The class
must still implement abstract members, but it may not override a final/default
interface member.

Qualified references use `namespace..symbol`; the left side must be an
imported namespace, not a class or interface name. `namespace::symbol`
remains accepted as a compatibility alias.

### Metadata and import

The contract model is carried through normal `.rxbin` metadata with:

- `META_INTERFACE`
- `META_IMPLEMENTS`
- `META_MEMBER`

That metadata is sufficient for import reconstruction of class/interface
headers without parsing procedure bodies. Imported stubs are not re-exported as
new local contracts, and richer imported stubs replace poorer duplicates.

### Runtime dispatch

Created objects carry their concrete class identity. The VM then resolves
contract calls through load/link-time registries:

- a method registry keyed by concrete class plus member name
- a factory-provider registry keyed by interface plus factory member name

`srcmethod` resolves the effective method for an interface/class receiver. The
registry prefers a concrete class method and otherwise falls back to a final
interface default method.

`srcfproc` resolves interface factories. Every candidate provider is evaluated
through its effective `match`; omitted `match` behaves as score `1`, scores
`<= 0` reject, highest positive score wins, and tied scores are broken
alphabetically by concrete class name.

## Source Tree and Parser Mode

cREXX now has an explicit split between the user-facing source model and the
mutable compiler tree.

- After the early source-shaping and source-location work, the compiler builds
  an immutable `SourceNode` tree in `compiler/rxcp_source_tree.c`.
- `context->source_tree` is the canonical user-facing tree for authored
  structure, diagnostics, semantic sidecars, metadata anchors, and editor
  projection.
- `context->ast` / `work_ast` remains the mutable compiler tree for import
  loading, exit dispatch, fixed-point rewrites, optimization, and emission.
- `ASTNode` instances keep explicit links back to the source tree so later
  rewritten nodes can still report against authored source.

Parser mode (`rxc --syntaxhighlight`) uses the same parser and early source
preparation, but it routes through `compiler/rxcp_highlight_controller.c` and
serializes DSLSH from `source_tree`, not from the later rewritten work tree.
The controller also keeps retained parser-mode cache state for imports and exit
discovery across requests.

For the compiler-side build order and tree-split details, see
[Parsing Pipeline Anatomy](../../compiler/docs/parsing_pipeline_anatomy.md).

For the DSLSH/editor mapping and parser-mode contract, see
[cREXX DSLSH Integration](../../compiler/docs/dslsh_integration.md).

## Core Data Structures

### `ASTNode` (from `compiler/rxcp_ast.h`)
The `ASTNode` forms the backbone of the compilation process, maintaining context, tree structure (parent/child/sibling relations), value/target typing details, code generation fragments, and parser token details.

```c
struct ASTNode {
    Context *context;
    int node_number;
    NodeType node_type;
    char* file_name;               
    ValueType value_type;    /* Value type */
    size_t value_dims;       /* Value dimensions */
    int *value_dim_base;     
    int *value_dim_elements; 
    char* value_class;       /* Value class name */
    int *target_dim_base;    
    int *target_dim_elements;
    ValueType target_type;   /* Target type */
    size_t target_dims;      /* Target dimensions */
    char* target_class;      /* Target class name */
    int high_ordinal; /* Order of node after validation but before optimisations */
    int low_ordinal;  /* lowest in this tree root */
    int register_num;
    char register_type;
    int additional_registers; 
    int num_additional_registers;
    char is_ref_arg;
    char is_opt_arg;
    char is_const_arg;
    char is_varg;
    ASTNode *free_list;
    ASTNode *parent, *child, *sibling;
    ASTNode *association; /* E.g. for LEAVE / ITERATE relevant DO node or LEAVE_WITH relevant BLOCK_EXPR */
    Token *token;
    Scope *scope;
    char *node_string;
    size_t node_string_length;
    char free_node_string;
    rxinteger int_value;
    int bool_value;
    double float_value;
    char* decimal_value; /* Decimal value as a string */
    int exit_obj_reg; /* VM register index of the attached Exit object */
    /* These are only valid after the set_source_location walker has run */
    Token *token_start, *token_end;
    char *source_start, *source_end;
    int line, column;
    SymbolNode *symbolNode;
    /* These are used by the code emitters */
    OutputFragment *output;          /* Primary node output or loop assign / init instruction */
    OutputFragment *cleanup;         /* Clean up logic */
    OutputFragment *loopstartchecks; /* Begin Loop exit checks */
    OutputFragment *loopinc;         /* Loop increments */
    OutputFragment *loopendchecks;   /* End Loop exit checks */
};
```

## Abstract Syntax Tree: Scope & Block Construction

### Mapping `DO` loops from Lemon Parser to AST
In the compiler, block scopes such as `DO` loops are strictly translated from Lemon grammar tokens into a parent-child AST topology.

**Example from `compiler/rxcpbgmr.y`:**
```yacc
tk_doloop(D)  ::= TK_DO(T).
                  { D = ast_f(context, DO, T); }
do(G)         ::= tk_doloop(T) dorep(R) TK_EOC instruction_list(I) TK_END.
                  { G = T; add_ast(G,R); add_ast(G,I); }
```

1. **Root Creation**: When a `TK_DO` token is identified, a new `ASTNode` of type `DO` is instantiated (via `ast_f`).
2. **Repetition and Conditions**: Sub-rules process the `dorep` (like `TO`, `BY`, `FOR` attributes) into a `REPEAT` AST node or `docond` (like `WHILE` / `UNTIL`).
3. **Block Body**: The `instruction_list(I)` contains all expressions and assignments defined within the loop body.
4. **Tree Assembly**: The `REPEAT` clause, `WHILE`/`UNTIL` conditions, and the actual loop body instructions are iteratively appended as **child nodes** to the parent `DO` node using the `add_ast(parent, child)` and `add_sbtr(older_sibling, younger_sibling)` C functions.
5. **Association**: The AST node also uses the `association` pointer to link commands like `LEAVE` or `ITERATE` directly back to the target enclosing `DO` loop node.

## Execution and the VM
Once compilation via `rxc` and `rxas` is complete, `rxvm` handles the execution. 
Modules are ingested into memory mapping via functions like `rxldmod`. The VM spins up its contexts, loading dynamically or statically linked extensions, and invokes `rxvm_run` to march through and execute the virtual CPU instructions matching the loaded byte sequence.
