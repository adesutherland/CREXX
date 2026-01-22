options levelb
/* Implicit Rewrites Test */

say "Testing Implicit Rewrites..."

/* SCONCAT: Verify that a = "Hello" "World" (no operator) compiles and executes as concatenation. */
/* Note: SCONCAT is converted to CONCAT if no gap, but the issue description says */
/* 'Verify that a = "Hello" "World" (no operator) compiles and executes as concatenation.' */
/* Usually "Hello" "World" with a space is SCONCAT, "Hello""World" is CONCAT. */
/* But rxcpbval.c says: if (node->child->sibling->source_start - node->child->source_end == 1) node->node_type = OP_CONCAT; */
/* That looks like it converts SCONCAT to CONCAT if the gap is exactly 1 (which a single space would be). */

a = 'Hello' || 'World'
say 'CONCAT test (a = "Hello" || "World"): ' || a
if a \= 'HelloWorld' then do
    say 'FAILED: CONCAT should result in "HelloWorld"'
    return 1
end

b = 'Hello' 'World'
say 'SCONCAT test (b = "Hello" "World"): ' || b
if b \= 'Hello World' then do
    say 'FAILED: SCONCAT should result in "Hello World"'
    return 1
end

/* Verify that SCONCAT and EXIT compile */
/* SCONCAT with no space is actually handled by the lexer as one string if same quotes, */
/* but different quotes might trigger SCONCAT in the parser. */
c = "Hello"'World'
say 'SCONCAT (diff quotes) test (c = "Hello"' || "'World'): " || c
if c \= 'HelloWorld' then do
    say 'FAILED: SCONCAT (diff quotes) should result in "HelloWorld"'
    return 1
end

say "Implicit Rewrites Test PASSED"
return 0
/* ADDRESS "SYSTEM" "echo ok" */
/* EXIT 0 */
