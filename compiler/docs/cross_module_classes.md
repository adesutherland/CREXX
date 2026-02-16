# Technical Design: Cross-Module Class Metadata

## 1. Overview
Currently, the cREXX compiler only exports procedure signatures to the `.rxbin` metadata. Class definitions (names, members, and register mappings) are not exported, making them invisible to other modules that `import` the binary. This document details the extension of the binary format and the compiler/assembler to support full class metadata export and import.

## 2. Metadata Binary Format Extensions

### 2.1 New Metadata Tags (`rxbin.h`)
We will add two new metadata tags to the `const_pool_type` enumeration in `binutils/include/rxbin.h`:
*   `META_CLASS`: Defines a class entry.
*   `META_ATTR`: Defines an attribute of a class (member variable).

### 2.2 New Metadata Structures (`rxbin.h`)
```c
typedef struct meta_class_constant {
    meta_entry base;
    size_t symbol; /* Fully qualified class name (String constant index) */
    size_t option; /* Options, e.g., "levelb" (String constant index) */
} meta_class_constant;

typedef struct meta_attr_constant {
    meta_entry base;
    size_t symbol; /* Fully qualified attribute name "Class.Attr" (String index) */
    size_t option; /* Options (String index) */
    size_t type;   /* Type name, e.g., ".int" (String index) */
    size_t reg;    /* Register index in the object instance */
} meta_attr_constant;
```

## 3. Assembler Changes (`rxas`)

### 3.1 New `.meta` Syntax
The assembler will be updated to support two new `.meta` directive forms:
1.  **Class Definition:** `.meta "ClassName" = "options" "" .class`
2.  **Attribute Definition:** `.meta "ClassName.AttrName" = "options" "type" .attr index`

### 3.2 Parser and Internal Representation
*   **`rxasgrmr.y`**: Add rules for `.class` and `.attr` tokens in `instruction ::= KW_META ...`.
*   **`rxas.h`**: Add `CLASS_META` and `ATTR_META` to `queue_item_type`.
*   **`rxas_opt.c` / `rxasassm.c`**: Implement bridge and implementation functions (`rxasmeclss`, `rxasmeattr`) to encode these into the binary constant pool.

## 4. Compiler Changes (`rxc`)

### 4.1 Emitter (`compiler/rxcp_emit_meta.c`)
The `meta_set_symbol` function will be extended:
*   **Classes:** When encountering a `CLASS_SYMBOL`, it will emit a `.meta ... .class` directive.
*   **Attributes:** When encountering a `VARIABLE_SYMBOL` whose parent scope is a `CLASS_DEF`, it will emit a `.meta ... .attr` directive instead of a standard register metadata directive. This ensures the importer knows the register mapping is relative to the object instance, not a global register.

### 4.2 Importer (`compiler/rxcpfunc.c`)
The `read_constant_pool_for_functions` function (and related metadata loaders) will be updated:
*   **`META_CLASS`**:
    1. Create a `CLASS_SYMBOL` in the import namespace scope.
    2. Create a child `Scope` for the class.
    3. Link the symbol to the scope (`defines_scope`).
*   **`META_ATTR`**:
    1. Parse the fully qualified name to find the parent `CLASS_SYMBOL`.
    2. Add a `VARIABLE_SYMBOL` to the class scope.
    3. Set the `register_num` and `type` based on the metadata.
*   **`META_FUNC` (Methods)**:
    1. If the function name starts with a known class name, add the `FUNCTION_SYMBOL` to that class's scope instead of the namespace scope.

## 5. Implementation Plan

### Phase 1: Infrastructure (Binary & Assembler)
1.  Modify `binutils/include/rxbin.h` to add `META_CLASS`, `META_ATTR` and their structs.
2.  Update `assembler/rxasgrmr.y` to recognize `.class` and `.attr` operands.
3.  Implement `rxasmeclss` and `rxasmeattr` in `assembler/rxasassm.c`.
4.  Implement queuing functions in `assembler/rxas_opt.c`.

### Phase 2: Export (Compiler Emitter)
1.  Update `compiler/rxcp_emit_meta.c` to detect `CLASS_SYMBOL` and class members.
2.  Emit the new `.meta` directives during code generation.

### Phase 3: Import (Compiler Importer)
1.  Modify `compiler/rxcpfunc.c` to process `META_CLASS` and `META_ATTR` during `import`.
2.  Implement logic to reconstruct the class hierarchy and member register mappings in the symbol table.

### Phase 4: Verification
1.  Create a test Rexx file defining a class.
2.  Compile and assemble it to `.rxbin`.
3.  Create a second Rexx file that imports the first and uses the class/attributes.
4.  Verify that the compiler correctly validates attribute access and method calls.
