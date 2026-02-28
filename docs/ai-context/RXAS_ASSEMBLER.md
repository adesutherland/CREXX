# cREXX Assembler Architecture (`rxas`)

The `rxas` assembler is responsible for translating human-readable Intermediate Representation (IR) assembly (`.rxas`) into packed executable bytecode (`.rxbin`) consumed by the `rxvm` interpreter.

## 1. Assembler Pipeline

The assembler processes source files through a pipelined, pseudo-two-pass architecture:

1. **Lexical Analysis (`re2c`)**:
   - Source defined in `assembler/rxasscan.re`.
   - Tokenizes input into REXX Assembly primitives: registers (`RREG`, `GREG`, `AREG`), literal types (`STRING`, `INT`, `FLOAT`, `DECIMAL`, `HEX`), symbols (`ID`, `LABEL`, `FUNC`), and assembler directives (e.g., `.locals`, `.globals`, `.expose`, `.meta`).

2. **Parsing (`Lemon`)**:
   - Grammar defined in `assembler/rxasgrmr.y`.
   - Enforces the structural integrity of the `.rxas` file (headers, function definitions, variable declarations, and instruction sequences).
   - The parser actions invoke Builder API functions directly (e.g., `rxasgen*`, `rxaslabl`, `rxasproc`), translating syntax rules into buffered internal data structures.

3. **In-Memory Buffering & Constant Pooling**:
   - Handled in `assembler/rxasassm.c`.
   - Instructions and inline constants are appended sequentially into a dynamic array of `bin_code` elements.
   - Complex types (Strings, Decimals, Procedure Headers, Metadata) are deduplicated via AVL Trees and injected into a variable-length **Constant Pool**.

4. **Backpatching & Optimization (Second Pass)**:
   - Forward references (branches to undefined labels, calls to undefined procedures) are logged as a linked list of references.
   - At the end of the parse, `backptch()` traverses all trees, resolves symbol addresses, updates the binary stream, and applies peephole jump optimizations.

## 2. Core Internal Data Structures

The state of the assembler is held within the `Assembler_Context` struct, specifically within the `context->binary` object, which mirrors the layout of the final `.rxbin` file.

### Instruction Stream
Instructions are flattened into an array of unions called `bin_code` (defined in `binutils/include/rxdefs.h`). An instruction is represented by an opcode element, immediately followed by elements representing its operands.
```c
// Example buffer size handling in gen_instr()
context->binary.binary[context->binary.inst_size].instruction.opcode = opcode;
context->binary.binary[context->binary.inst_size++].instruction.no_ops = operands;

// Operand elements
context->binary.binary[context->binary.inst_size++].index = get_reg_number(...);
context->binary.binary[context->binary.inst_size++].iconst = token->integer;
```

### Constant Pool
Strings, procedure mappings, debug metadata, and exported symbols are packed into `const_pool`. This is a sequential buffer of dynamically sized records. Every record starts with a `chameleon_constant` header dictating its type and byte size.
Types include: `STRING_CONST`, `PROC_CONST`, `EXPOSE_REG_CONST`, `EXPOSE_PROC_CONST`, `META_FUNC`, `META_REG`, etc.

### Symbol Tracking (AVL Trees)
To deduplicate constants and resolve identifiers in `O(log N)` time, the assembler leverages a custom AVL tree implementation (`avl_tree.h`). Active trees include:
- `string_constants_tree`
- `decimal_constants_tree`
- `binary_constants_tree`
- `label_constants_tree`
- `proc_constants_tree`
- `extern_constants_tree`

## 3. Two-Pass Resolution (Backpatching)

Assembly requires a two-pass approach because a jump or call can reference a label or procedure defined further down in the file. `rxasassm.c` handles this elegantly by building a `struct backpatching` header for every symbol encountered:

```c
struct backpatching {
    int defined;          // 1 if definition encountered, 0 if only referenced
    size_t index;         // Final resolved target index in the binary or const_pool
    struct backpatching_references *refs; // Linked list of unresolved usages
};

struct backpatching_references {
    size_t index;         // The instruction operand index in the bin_code array
    Assembler_Token *token; 
    struct backpatching_references *link;
};
```

When the `EOF` is reached, the `backptch(Assembler_Context *context)` function orchestrates the final resolution:

1. **`optimise_labels()`**: Performs peephole optimization. If a label simply targets an unconditional branch (`br`), the target of the label is recursively collapsed to point directly at the final destination, saving execution cycles.
2. **`backpatch_procedures()`**: Walks the `proc_constants_tree`. Raises errors for any procedures referenced but never defined. Resolves valid procedures.
3. **`backpatch_labels()`**: Walks the `label_constants_tree`. Updates the placeholder `bin_code` indices mapped in the `refs` linked list with the actual instruction offsets.

## 4. Binary Emission (`.rxbin`)

Once parsing and backpatching conclude without errors, the assembler flushes the `Assembler_Context` state into a highly compressed `.rxbin` file.

The structural output consists of:
1. **Magic Header & Version**: Validates the file format.
2. **Global Counters**: e.g., Number of global registers (`context->binary.globals`).
3. **The Constant Pool**: The exact byte stream accumulated in `context->binary.const_pool`, which includes linked-list pointers mapping exposed exports and metadata objects.
4. **The Bytecode Stream**: The `context->binary.binary` sequence, representing the fully resolved executable operations.