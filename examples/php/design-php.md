PHP Phase 1 — Design for Harry
1. Objective
   Implement a small PHP-to-cREXX translator using:
   Level L grammar
   ↓
   PHP parse tree
   ↓
   Level B reader
   ↓
   generated cREXX source

The purpose is to prove the complete pipeline, not to support PHP applications such as WordPress.
The result should translate and execute small procedural PHP programs.
2. Supported PHP subset
   Phase 1 supports:
- <?php opening tag;
- optional ?> closing tag;
- scalar variables;
- integer and real numbers;
- simple single- and double-quoted strings;
- assignment;
- arithmetic expressions;
- comparison expressions;
- echo;
- if;
- else;
- while;
- braces;
- semicolons;
- parentheses.
  Example:
<?php
$a = 10;
$b = 20;
$c = $a + $b;

if ($c > 20) {
    echo "Result:";
    echo $c;
}

while ($a < 13) {
    echo $a;
    $a = $a + 1;
}
?>

3. Explicit exclusions
   Do not implement these in Phase 1:
- arrays;
- associative arrays;
- functions;
- classes and objects;
- namespaces;
- includes and requires;
- closures;
- exceptions;
- references;
- interpolated strings;
- built-in PHP functions;
- database access;
- HTTP or web-server variables;
- Composer or framework support;
- PHP type declarations;
- foreach;
- switch;
- for;
- do ... while.
  These features should be added only after the first pipeline works.
4. Variable mapping
   PHP variables begin with $:
   $total = 10;

The reader generates the equivalent cREXX variable without $:
total = 10

Phase 1 should treat PHP variable names as simple identifiers. Variable names are case-insensitive after translation, following normal cREXX behaviour.
5. Output mapping
   For Phase 1:
   echo expression;

is translated to:
say expression

This deliberately provides line-oriented output. PHP’s exact output semantics, including output without an automatic newline, can be addressed later.
For the first version, support one expression per echo statement:
echo $total;
echo "Total:";

Do not support comma-separated echo arguments yet.
6. Expression mapping
   The grammar should support:
+  -  *  /
         .              string concatenation
         == != < <= > >=
         ( )

PHP concatenation:
$name . " Smith"

maps to cREXX concatenation:
name || " Smith"

The reader must generate the appropriate cREXX operator.
7. Control-flow mapping
   PHP:
   if ($a > 10) {
   echo $a;
   } else {
   echo 0;
   }

becomes:
if a > 10 then do
say a
end
else do
say 0
end

PHP:
while ($a < 10) {
$a = $a + 1;
}

becomes:
do while a < 10
a = a + 1
end

The reader should generate indentation for readability, although indentation is not semantically required by cREXX.
8. Grammar responsibilities
   The Level L grammar must:
1. recognise the PHP opening tag;
2. recognise variables, literals, operators, and punctuation;
3. parse assignments;
4. parse echo;
5. parse if / else;
6. parse while;
7. parse nested blocks;
8. produce stable, named parse-tree nodes.
   The grammar must not:
- evaluate expressions;
- resolve variable values;
- generate cREXX;
- decide how PHP semantics are implemented.
9. Reader responsibilities
   The Level B reader must:
1. load the generated PHP grammar;
2. parse a .php source file;
3. inspect the parse tree;
4. generate cREXX source;
5. remove the $ prefix from variable names;
6. convert PHP concatenation to cREXX concatenation;
7. translate control structures;
8. report unsupported constructs clearly;
9. write the generated .crexx file.
   Example diagnostic:
   PHP Phase 1 error:
   unsupported construct: array
   source line: 12

10. Suggested project structure
    examples/php/
    grammar/
    php_phase1.levell
    php_reader.crexx
    samples/
    hello.php
    arithmetic.php
    control.php
    build/
    php.build
    README.md

Generated grammar and compiled binaries should remain build artifacts.
11. Initial test programs
    hello.php
<?php
echo "Hello from PHP";
?>

Expected generated cREXX:
say "Hello from PHP"

arithmetic.php
<?php
$a = 10;
$b = 20;
$c = $a + $b;
echo $c;
?>

Expected output:
30

control.php
<?php
$i = 1;

while ($i <= 3) {
    echo $i;
    $i = $i + 1;
}
?>

Expected output:
1
2
3

conditional.php
<?php
$value = 42;

if ($value > 40) {
    echo "greater";
} else {
    echo "smaller";
}
?>

Expected output:
greater

12. Acceptance criteria
    Phase 1 is complete when:
- the grammar builds successfully;
- the reader builds successfully;
- all four sample programs parse;
- generated cREXX is readable;
- generated cREXX compiles and runs;
- nested if and while blocks work;
- unsupported syntax produces a useful error;
- the complete process is described in the README.
13. Next phase
    Only after Phase 1 is stable should we consider adding:
1. functions and return;
2. arrays;
3. built-in functions;
4. for and foreach;
5. improved PHP output semantics;
6. more accurate PHP type conversion.
   The first milestone is therefore not “PHP support”. It is:
   A working Level L grammar and cREXX reader that translate a small, recognisable procedural PHP subset.

14. Current Phase 1 implementation
   The first reader implementation is now present in `php_reader.crexx`.
   It uses the generated `phpgrammar` module and the standard `lllanguage` /
   `lltree` APIs to emit readable cREXX source. `migration.php` is the first
   migration example and covers variables, arithmetic, concatenation, `echo`,
   `if/else`, and `while`.

   Focused validation on 24 October 2026 generated and assembled the grammar,
   compiled and linked the reader, translated `migration.php`, and compiled
   and ran the generated cREXX. The observed output was:

       Customer: Harry
       Orders: 3
       Processing order 1
       Processing order 2
       Processing order 3

15. Phase 3 — Functions
    The first function slice adds user-defined procedural functions with scalar
    parameters and expression returns. PHP declarations are emitted as ordinary
    cREXX procedures; calls remain ordinary cREXX calls. Function declarations
    are emitted before the generated top-level program so PHP declaration order
    does not affect calls.

    Supported first slice:
    - `function name($arg1, $arg2) { ... }`;
    - scalar parameters;
    - `return expression;`;
    - calls used in expressions, such as `name($value, 2)`.

    Explicitly deferred: defaults, type hints, references, variadics, closures,
    nested functions, generators, and built-in PHP functions.

    - **PHP-FUNCTION-AC01:** a named function parses and becomes a cREXX
      procedure with typed scalar arguments;
    - **PHP-FUNCTION-AC02:** return expressions and calls produce the expected
      result;
    - **PHP-FUNCTION-AC03:** existing array and scalar migrations remain valid;
    - **PHP-FUNCTION-AC04:** unsupported function forms remain outside the
      grammar and produce a parse diagnostic.

    The first implementation is in `php.levell` and `php_reader.crexx`, with
    `php-functions-sample.php` as the focused migration fixture.

16. Phase 4 — String functions
    The first built-in string-function slice maps common PHP calls to existing
    cREXX string functions and to two small PHP runtime adapters where the
    conventions differ.

    Supported functions:
    - `strlen`, `strtoupper`, `strtolower`, `trim`, `ltrim`, `rtrim`, and
      `strrev`;
    - `substr` with non-negative start offsets;
    - string-only `str_replace`;
    - `strpos`, returning a zero-based integer or `-1` when absent.

    PHP arrays as string-function arguments, PHP's `false`/`null` distinction,
    negative `substr` offsets, and array replacement/search operands remain
    deferred. The adapters are owned by `php_runtime`.

    - **PHP-STRING-AC01:** supported string calls parse and generate readable
      cREXX calls;
    - **PHP-STRING-AC02:** case, length, trimming, reverse, substring,
      replacement, and position results execute as documented;
    - **PHP-STRING-AC03:** the existing scalar, function, and array samples
      remain valid.

    The focused fixture is `php-string-sample.php`.

17. Phase 5 — Numeric functions
    The first numeric slice supports scalar numeric arguments through existing
    cREXX numeric functions and small PHP runtime adapters:
    `abs`, `min`, `max`, `intval`, `floatval`, `round`, `ceil`, and `floor`.
    PHP array arguments and exact PHP conversion of booleans, null, and invalid
    numeric strings remain outside this slice.

    - **PHP-NUMERIC-AC01:** supported numeric calls parse and generate cREXX;
    - **PHP-NUMERIC-AC02:** integer conversion, floating conversion, extrema,
      absolute value, rounding, ceiling, and floor execute correctly;
    - **PHP-NUMERIC-AC03:** existing PHP samples remain valid.

    The focused fixture is `php-numeric-sample.php`.

18. Named grouping-node correction
    The named tree preserves the grammar's `group.value` rule, including its
    ordered opening token, expression and closing token. The reader must retain
    those parentheses rather than treating that node as a binary expression.

- **PHP-GROUP-AC01 — done:** the real reader emitter preserves a three-child
  `group.value` and nested groups with exact parenthesized output. One optimized
  product-VM synthetic-tree check uses the existing generic tree runtime; no PHP
  grammar generation or language expansion is required.
- **PHP-GROUP-STEP01 — done:** match the authored node name and compose the
  unchanged reader expression procedures into the owned regression fixture.
- **PHP-GROUP-STEP02 — done:** qualify that focused control, including a private
  before-copy negative, then retain it in ordinary QA. This does not complete
  the broader Phase 1 acceptance criteria above.
