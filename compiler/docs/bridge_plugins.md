# cREXX Bridge Plugins: Implementation Guide

Bridge Plugins allow the cREXX compiler to execute Rexx code during the validation phase. This enables custom syntax transformations, macro-like expansions, and domain-specific optimizations by intercepting unrecognized commands (`IMPLICIT_CMD`).

## 1. Overview

When the compiler encounters a statement that is not a recognized Rexx keyword or a resolved function/method call, it creates an `IMPLICIT_CMD` node. The **Fixpoint Orchestrator** then dispatches these nodes to registered Bridge Plugins.

A Bridge Plugin can:
1.  **Analyze**: Inspect the tokens of the command.
2.  **Negotiate**: Wait for more information (e.g., symbol resolution).
3.  **Inject**: Return a string of Rexx source code to replace the command.
4.  **Decline**: Return an empty string, letting the compiler default to `ADDRESS SYSTEM`.

## 2. Writing a Plugin

Plugins are written in cREXX (Level B). They must define a specific class structure to map the compiler's internal token representation.

### 2.1 The Token Class
To access token data, the plugin must define a `token` class using physical register mapping (`WITH`).

```rexx
/* Token structure used for interop with the Bridge */
token: class
    /* Standard attributes mapped to VM registers */
    type    = .int    with register.1
    subtype = .int    with register.2
    text    = .string with register.3
    line    = .int    with register.4
    column  = .int    with register.5
    length  = .int    with register.6
    
    get_text: method = .string
        return text
```

### 2.2 The Entry Point
The plugin must expose a procedure that accepts an array of tokens and returns a string (the replacement code).

```rexx
/* Entry point: takes an array of .token objects */
my_handler: procedure = .string
    arg tokens = .token[]
    
    /* Example: Intercept the "TODO" command */
    if tokens.0 >= 1 & tokens[1].get_text() == "TODO" then do
        /* Extract the message if provided */
        msg = ""
        if tokens.0 >= 2 then msg = tokens[2].get_text()
        
        /* Inject a warning message into the source */
        return "SAY 'STUB: " || msg || "'"
    end

    /* Return "" to let the compiler handle it normally */
    return ""
```

## 3. Deployment & Usage

### 3.1 Compiling the Plugin
Plugins are compiled to `.rxbin` and loaded by the compiler at runtime.

```bash
rxc -o my_plugin my_plugin.rexx
rxas -o my_plugin.rxbin my_plugin.rxas
```

### 3.2 Invoking from Source
Any unrecognized statement in a consumer script will trigger the plugin dispatch.

**Consumer (`test.rexx`):**
```rexx
options levelb
TODO "Implement file I/O"
```

**Compiler Dispatch:**
1.  `TODO` is unrecognized -> `IMPLICIT_CMD`.
2.  `my_plugin` is called with tokens `["TODO", "Implement file I/O"]`.
3.  Plugin returns `"SAY 'STUB: Implement file I/O'"`
4.  Compiler parses and injects the `SAY` instruction.

## 4. Technical Details

### 4.1 Memory Management
The compiler's Bridge (`rxcp_val_plugin.c`) manages the lifetime of the injected code. It creates a temporary `Context` for the parsed fragment and retains it until the master compilation finishes to ensure tokens and strings remain valid.

### 4.2 Fixpoint Interaction
If a plugin injects code that introduces new variables or functions, the compiler's **Fixpoint Loop** automatically runs another iteration to resolve them. This ensures that injected code is just as type-safe as handwritten code.

### 4.3 Mapping Reference

The `token` class can map the following fields from the compiler:

| Field | Index | Type | Description |
| :--- | :--- | :--- | :--- |
| `type` | 1 | `.int` | The raw Lexer token type. |
| `subtype` | 2 | `.int` | Lexer subtype. |
| `text` | 3 | `.string` | The string literal from the source. |
| `line` | 4 | `.int` | Source line. |
| `column` | 5 | `.int` | Source column. |
| `length` | 6 | `.int` | Token length in characters. |
| `file` | 7 | `.string` | Source file name. |
| `node_type`| 8 | `.int` | AST node type. |
| `sym_name` | 11 | `.string` | Resolved symbol name (if any). |
| `sym_type` | 12 | `.int` | Symbol type (Variable, Function, etc). |
