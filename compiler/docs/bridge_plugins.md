# cREXX Bridge Plugins: Implementation Guide

Bridge Plugins allow the cREXX compiler to execute Rexx code during the validation phase. This enables custom syntax transformations, macro-like expansions, and domain-specific optimizations by intercepting unrecognized commands (`IMPLICIT_CMD`).

## 1. Overview

When the compiler encounters a statement that is not a recognized Rexx keyword or a resolved function/method call, it creates an `IMPLICIT_CMD` node. The **Fixpoint Orchestrator** then dispatches these nodes to registered Bridge Plugins.

Modern cREXX plugins are **stateful**. The compiler instantiates plugin classes once at the start of compilation and dispatches commands to these instances. This allows plugins to maintain state (e.g., counters, cross-instruction analysis) across the entire compilation unit.

A Bridge Plugin can:
1.  **Analyze**: Inspect the tokens of the command.
2.  **Negotiate**: Wait for more information (e.g., symbol resolution).
3.  **Inject**: Return a string of Rexx source code to replace the command.
4.  **Decline**: Return an empty string, letting the compiler default to `ADDRESS SYSTEM`.
5.  **Cleanup**: Perform final logging or cleanup during the `shutdown` phase.

## 2. Writing a Plugin

Plugins are written in cREXX (Level B) and must reside in the `rxcplugin` namespace.

### 2.1 The Token Class

To access token data, the plugin bundle must define a `token` class using physical register mapping. Because cREXX attributes are private by default, accessor methods are required.

```rexx
namespace rxcplugin

token: class
    /* Standard attributes mapped to VM registers */
    type      = .int    with register.1.int
    subtype   = .int    with register.2.int
    text      = .string with register.3.string
    line      = .int    with register.4.int
    column    = .int    with register.5.int
    length    = .int    with register.6.int
    file      = .string with register.7.string
    node_type = .int    with register.8.int
    /* ... additional fields ... */
    
    get_text: method = .string
        return text

    get_node_type: method = .int
        return node_type
```

### 2.2 The Plugin Class

A plugin is a class in the `rxcplugin` namespace. The compiler discovers it by looking for the class factory.

```rexx
namespace rxcplugin

MyOptimizer: class
    count = .int

    /* Factory: Called once by the compiler to instantiate the plugin */
    *: factory
        count = 0
        return

    /* process: Called for every IMPLICIT_CMD encountered */
    process: method = .string
        arg tokens = .token[]
        
        if tokens.0 >= 1 & tokens[1].get_text() = "MY_CMD" then do
            count = count + 1
            return "SAY 'Intercepted MY_CMD'"
        end

        return ""

    /* shutdown: Called once at the end of compilation */
    shutdown: method
        say "MyOptimizer: Processed" count "commands."
        return
```

## 3. Deployment & Discovery

### 3.1 Bundling

Currently, all plugin classes and the `token` definition should be consolidated into a single Rexx file (e.g., `crexx_plugins.rexx`) and compiled into a single binary bundle.

```bash
rxc -n -o crexx_plugins crexx_plugins.rexx
rxas -o crexx_plugins crexx_plugins
```

The resulting `crexx_plugins.rxbin` must be placed in the compiler's search path (typically the same directory as the `rxc` executable or the current working directory).

### 3.2 Discovery Mechanism

The compiler bridge automatically:
1.  Loads `crexx_plugins.rxbin`.
2.  Scans the internal symbol table for any procedure ending in `.§factory` within the `rxcplugin.` namespace.
3.  Calls the factory to create a persistent instance.
4.  Identifies the `.process` and `.shutdown` methods for that class.

## 4. Technical Details

### 4.1 Memory Management
The compiler's Bridge (`rxcp_val_plugin.c`) manages the lifetime of the injected code. It creates a temporary `Context` for the parsed fragment and retains it until the master compilation finishes to ensure tokens and strings remain valid.

### 4.2 Fixpoint Interaction
If a plugin injects code that introduces new variables or functions, the compiler's **Fixpoint Loop** automatically runs another iteration to resolve them.

### 4.3 Mapping Reference

The `token` class maps the following fields via `WITH REGISTER.x`:

| Field | Register | Type | Description |
| :--- | :--- | :--- | :--- |
| `type` | 1 | `.int` | The raw Lexer token type. |
| `subtype` | 2 | `.int` | Lexer subtype. |
| `text` | 3 | `.string` | The string literal from the source. |
| `line` | 4 | `.int` | Source line. |
| `column` | 5 | `.int` | Source column. |
| `length` | 6 | `.int` | Token length in characters. |
| `file` | 7 | `.string` | Source file name. |
| `node_type`| 8 | `.int` | AST node type. |
| `ord_low` | 9 | `.int` | Token order (low). |
| `ord_high` | 10 | `.int` | Token order (high). |
| `sym_name` | 11 | `.string` | Resolved symbol name (if any). |
| `sym_type` | 12 | `.int` | Symbol type (Variable, Function, etc). |
