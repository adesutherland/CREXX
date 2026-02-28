# cREXX Architecture

`crexx` is a custom REXX-to-bytecode toolchain that translates Classic REXX semantics into an optimized bytecode format executed by a specialized VM. The process happens through three main binaries:
1. `rxc` - The Compiler
2. `rxas` - The Assembler
3. `rxvm` - The Virtual Machine (Interpreter)

## The Compilation Pipeline

The pipeline of transforming REXX source code into executable bytecode is structured as follows:

1. **re2c (Lexical Analyzer)**
   - Used to generate the scanner/lexer from `.re` rules (e.g., `compiler/rxcpbscn.re` and `assembler/rxasscan.re`).
   - Converts the raw REXX source text into a stream of discrete tokens (`Token` structs).

2. **Lemon (Parser Generator)**
   - The token stream is parsed using a grammar defined in `Lemon` (e.g., `compiler/rxcpbgmr.y` and `compiler/rxcpopgr.y`).
   - Lemon applies the grammar rules to recognize the syntactic structures of the REXX language and translates them into an Abstract Syntax Tree (AST).

3. **AST (Abstract Syntax Tree)**
   - The primary data structure bridging the parser and the code emitter.
   - Built using a hierarchical structure of `ASTNode` C structs that capture operations, scopes, typing, and tree associations.

4. **Emitter (IR -> Assembly)**
   - AST walkers (e.g., `rxcp_ast_walk.c`, `rxcp_emit_*.c`) traverse the tree.
   - Outputs intermediate string fragments representing the `rxas` Assembly instructions.

5. **Assembler (`rxas`)**
   - Parses the generated `rxas` Assembly instructions.
   - Translates human-readable IR assembly into packed binary format (`rxbin` bytecode).
   - Generates the final executable bytecode file.

6. **Interpreter (`rxvm`)**
   - `rxvm` reads and executes the `rxbin` bytecode.
   - Modules are loaded via `rxldmod`.
   - The execution loop happens inside the `rxvm_run` function (e.g., in `rxvmmain.c` / `rxvmintp.c`).

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
    ASTNode *association; /* E.g. for LEAVE / ITERATE TO relevant DO node */
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