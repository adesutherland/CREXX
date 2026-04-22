# cREXX Virtual Machine (Interpreter) Architecture

The `rxvm` interpreter is the runtime component of the `crexx` toolchain. It loads, links, and executes the compiled `.rxbin` bytecode. Its design emphasizes performance through direct threaded code (computed gotos), aggressive stack frame recycling, and an optimized value struct to handle REXX dynamic typing.

## 1. VM Lifecycle

The execution of a program within `rxvm` is handled in discrete phases (as defined in `inc/rxvm.h`):
1. **Creation**: `rxvm_create()` allocates the root `rxvm_context`.
2. **Loading**: `rxvm_load()` ingests a `.rxbin` binary file, loading it into an internal `module` struct, resolving the constant pool and the bytecode instruction stream.
3. **Linking**: `rxvm_link()` traverses newly loaded modules to resolve exports and external imports into a unified memory map. The call is now dirty-checked, so repeated bridge/runtime entry points become fast no-ops when no module state changed.
4. **Preparation**: `rxvm_prepare()` optionally patches the bytecodes into direct threading pointers for maximum speed.
5. **Execution**: `rxvm_run()` / `rxvm_call()` invoke a target procedure (typically `main`) and launch the main interpreter loop.

## 2. Core Internal Structs

### `rxvm_context`
The root state of the VM environment. It houses the loaded modules, global configuration, and debugging state.
```c
typedef struct rxvm_context {
    char *location;
    size_t num_modules;
    module **modules;
    struct avl_tree_node *exposed_proc_tree;
    struct avl_tree_node *exposed_reg_tree;
    char link_dirty;
    char interface_method_registry_dirty;
    char interface_factory_registry_dirty;
    char debug_mode;
    // ...
} rxvm_context;
```

`link_dirty` is raised when new modules are loaded. The separate
`interface_method_registry_dirty` and `interface_factory_registry_dirty` flags
track when the interface method and factory caches need rebuilding. This keeps
repeated `rxvm_link()` calls cheap while still supporting late module loading.

### `stack_frame`
To minimize heap allocation overhead, the VM uses a custom call stack model. `stack_frame` structs maintain scope, local variables, and return state. When a function returns, the `stack_frame` is not immediately freed; it is placed onto a `frame_free_list` associated with the procedure, allowing the VM to rapidly reuse stack blocks for repeated calls.

```c
struct stack_frame {
    stack_frame *prev_free;          /* Pointer to next free recycled frame */
    stack_frame *parent;             /* Caller stack frame */
    proc_constant *procedure;        /* Executing procedure metadata */
    bin_code *return_pc;             /* Program Counter to return to */
    value *return_reg;               /* Target register for return values */
    size_t number_locals;            /* Number of local registers */
    interrupt_entry interrupt_table[RXSIGNAL_MAX]; /* Signal / Exception handlers */
    value **baselocals;              /* Array of initial / fixed pointers */
    value **locals;                  /* Active pointer map to variable values */
};
```

### `value` (Dynamic Typing Representation)
Classic REXX is a dynamically typed language where "everything is a string" conceptually, but performance dictates native type usage when possible. The `value` struct (from `interpreter/rxvalue.h`) is a polymorphic container storing a REXX variable's state. 

To limit memory fragmentation, strings shorter than `SMALLEST_STRING_BUFFER_LENGTH` (32 bytes) are stored directly inside the struct via `small_string_buffer` rather than forcing a heap allocation.

```c
struct value {
    value_type status;               /* Bit flag tracker for types */
    rxinteger int_value;             /* 64-bit/32-bit native integer */
    double float_value;              /* Native floating point */
    
    void *decimal_value;             /* Pointer for arbitrary precision math */
    size_t decimal_value_length;
    
    char *string_value;              /* String payload pointer */
    size_t string_length;
    
    char *binary_value;
    size_t binary_length;

    const char *object_type_name;    /* Runtime concrete class name */
    size_t object_type_name_length;
    
    value **attributes;              /* For associative arrays/objects */
    size_t num_attributes;

    /* Inline memory buffer to save mallocs on small strings */
    char small_string_buffer[SMALLEST_STRING_BUFFER_LENGTH]; 
};
```
Variables (`locals` arrays) consist of arrays of `value*` pointers managed strictly by the VM frames. There is no automated background Garbage Collector (GC). Instead, frame-bound variables are deterministically cleared (`clear_value`) and memory released when a `stack_frame` dies and exits scope.

The two `object_type_name` fields are the current Level B hook for interface
dispatch. Class factories stamp object values with `setobjtype`, and later VM
lookups use that concrete class identity when resolving interface member calls.

## 3. The Execution Loop

The core execution engine lives in `run()` within `interpreter/rxvmintp.c`. 

### Threaded vs Bytecode Dispatch
The VM uses conditional compilation (`#ifdef NTHREADED`) to flip between two execution models:
1. **Direct Threading (Default/Fast Mode)**: During the "Preparation" phase, opcodes are statically replaced with C `void*` pointers targeting the exact `&&label` implementing the opcode instruction. The instruction dispatch reduces to an incredibly fast computed goto: `goto *next_inst;`.
2. **Standard Bytecode Mode (`NTHREADED`)**: Operates via a massive standard C `switch(opcode)` statement wrapped in a while loop.

### Dispatch Macros
Instructions are executed via macro-driven blocks. For example, moving to the next instruction looks like:

```c
#define CALC_DISPATCH(n) { next_pc = pc + (n) + 1; next_inst = (next_pc)->impl_address; }
#define DISPATCH         { pc = next_pc; goto *(interrupts && !current_frame->is_interrupt)?&&INTERRUPT:next_inst; }
```
`DISPATCH` actively checks a global `interrupts` bit-flag to immediately branch into signal exception handling if an error occurred natively.

### Instruction Flow Example
Inside the `run()` loop, implementations are declared using `START_INSTRUCTION`. The assembler passes operands inline sequentially in the binary array.
```c
        START_INSTRUCTION(IADD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IADD R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI + op3RI)
            DISPATCH
```
In this example:
- `CALC_DISPATCH(3)` specifies that this instruction consumes 3 operand blocks.
- `op2RI` grabs the integer struct value mapped to Operand 2.
- `REG_RETURN_INT` maps the result back into the memory of Operand 1.
- `DISPATCH` safely jumps the Program Counter (`pc`) to the next instruction.

### Pooled float operands

As of `rxbin` format `002`, float literals are loaded from the constant pool
instead of being stored inline in operand slots. The bytecode still keeps the
same instruction formats (`FMT_F`, `FMT_R_F`, `FMT_R_R_F`, etc.), but the
operand slot now contains an index into a `FLOAT_CONST` record in
`const_pool`, and the interpreter resolves that record when a float operand is
read.

## 4. Current Interface Dispatch in the VM

The current Level B interface runtime slice adds three VM-facing pieces on top
of the older object model:

- `SETOBJTYPE_REG_STRING` stores a concrete class name on an object value
- `SRCMETHOD_REG_REG_STRING` resolves the effective method procedure from
  `object_type_name + member_name`
- `SRCFPROC_REG_STRING_REG` resolves an interface factory provider for either
  `interface_name` or `interface_name..factory_name`
- `TYPEOF_REG_REG` returns the canonical source type name of an object value
- `ISTYPE_REG_REG_STRING` tests an object value against an interface, class,
  or `.object`
- `ASSERTTYPE_REG_STRING` raises `CONVERSION_ERROR` on a failed object cast

`srcmethod` and `srcfproc` both return a `proc_constant *` in a normal
register, and the existing `dcall` path performs the actual invocation.

### Current `srcmethod` semantics

The current implementation is now:

- it rebuilds an interface-method registry only when newly loaded modules
  invalidate that cache
- registry rows are keyed by fully qualified concrete class name plus member
  name
- for each `class implements interface` link, the VM resolves the effective
  procedure for each interface member during link
- if a concrete `class.member` procedure exists, that wins
- otherwise, if the interface member kind is `method final`, the VM binds the
  interface's emitted default-body procedure instead
- if no registry row exists, `srcmethod` still falls back to a direct
  `class.member` lookup before raising `FUNCTION_NOT_FOUND`

### Current `srcfproc` semantics

The current implementation is now:

- it handles both the default `*` interface factory surface and named factory
  selectors
- it rebuilds a factory-provider registry only when newly loaded modules
  invalidate that cache
- registry rows are keyed by interface FQN plus factory member name
- for each candidate class, it resolves the concrete `§factory` or
  `§factory.member` procedure through the existing metadata/procedure tables
- each candidate row may also carry an optional resolved `§match` or
  `§match.member` procedure
- `srcfproc` calls the effective `match` on every candidate with the same
  argument list that will later be passed to the selected factory, even when
  only one candidate exists
- if a candidate has no explicit `match`, the VM behaves as if it had an
  implicit `match` returning `1`
- candidates scoring `<= 0` are rejected
- the highest positive score wins
- ties are broken alphabetically by fully qualified concrete class name
- if no provider exists, the VM raises `FUNCTION_NOT_FOUND`

Runtime module loading matters here as well. `METALOADMODULE` marks the
VM link state dirty, and the next `srcfproc` resolution forces
`rxvm_link()` before consulting the interface-factory registry. That
preserves correctness without paying the full relink cost on every
bridge call or factory lookup.
