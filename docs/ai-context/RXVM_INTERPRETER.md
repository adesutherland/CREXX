# cREXX Virtual Machine (Interpreter) Architecture

The `rxvm` interpreter is the runtime component of the `crexx` toolchain. It loads, links, and executes the compiled `.rxbin` bytecode. Its design emphasizes performance through direct threaded code (computed gotos), aggressive stack frame recycling, and an optimized value struct to handle REXX dynamic typing.

## 1. VM Lifecycle

The execution of a program within `rxvm` is handled in discrete phases (as defined in `inc/rxvm.h`):
1. **Creation**: `rxvm_create()` allocates the root `rxvm_context`.
2. **Loading**: `rxvm_load()` ingests a `.rxbin` binary file, loading it into an internal `module` struct, resolving the constant pool and the bytecode instruction stream.
3. **Linking**: `rxvm_link()` traverses multiple loaded modules to resolve exports and external imports into a unified memory map.
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
    char debug_mode;
    // ...
} rxvm_context;
```

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
    
    value **attributes;              /* For associative arrays/objects */
    size_t num_attributes;

    /* Inline memory buffer to save mallocs on small strings */
    char small_string_buffer[SMALLEST_STRING_BUFFER_LENGTH]; 
};
```
Variables (`locals` arrays) consist of arrays of `value*` pointers managed strictly by the VM frames. There is no automated background Garbage Collector (GC). Instead, frame-bound variables are deterministically cleared (`clear_value`) and memory released when a `stack_frame` dies and exits scope.

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