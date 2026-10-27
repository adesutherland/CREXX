# Phase 1 PHP-to-cREXX converter

This example is the first source-to-source migration slice described in
[`design-php.md`](design-php.md). The Level L grammar defines the supported PHP
syntax; [`php_reader.crexx`](php_reader.crexx) walks the generated parse tree
and writes readable cREXX source.

[`migration.php`](migration.php) exercises scalar variables, arithmetic,
string concatenation, `echo`, `if/else`, and `while`.

[`php-array-sample.php`](php-array-sample.php) exercises list and associative
arrays, lookup, `count()`, and insertion-order `foreach`.

[`php-functions-sample.php`](php-functions-sample.php) exercises user-defined
functions and scalar returns. [`php-string-sample.php`](php-string-sample.php)
exercises the first built-in string-function compatibility slice.

[`php-numeric-sample.php`](php-numeric-sample.php) exercises the first numeric
function compatibility slice.

## Build and run

From the repository root, prepare the driver and build-plan runner in the same
configured toolchain, then run the maintained plan:

```sh
cmake --build cmake-build-debug --target crexx rxbuild --parallel 4
cmake-build-debug/bin/crexx examples/php/setup.build
```

Use the configured build directory and executable suffix appropriate to your
platform. The equivalent direct invocation is
`cmake-build-debug/bin/rxbuild examples/php/setup.build`. The `crexx` convenience
form accepts the single `.build` filename without additional driver options.
Plan-relative paths resolve from the repository root.

The plan builds the grammar, array runtime and reader, translates the four
scalar/function/string/numeric samples, then runs those four applications.
Products stay under `.crexx-build/php`; translated cREXX sources stay under
`.crexx-build/.rxbuild-generated`. The reader replaces its output on each
conversion. Generated sources and binaries are build artifacts, not source
assets. The array migration sample is separate from these four plan inputs.

The sample lines show `expected=... result=...` for inspection. Those labels
are not runtime assertions: successful execution alone does not check the
values. The maintained grouping control separately checks parenthesized reader
output against an independent tree.

## Embedded source example

[`php_embedded.rxpp`](php_embedded.rxpp) demonstrates keeping a PHP example
visible inside an RXPP member. `##DALIAS PHP` collects the `PHP ... END PHP`
block into an array and calls `exec_to_process`. The sample callback only
exports the collected text to `php_embedded_source.php`; it deliberately does
not launch a nested VM or choose a PHP reader. A separate PHP conversion
service will own that process boundary.

Run the RXPP sample normally; it only demonstrates collection and export:

```powershell
crexx.exe examples/php/php_embedded.rxpp
```

To convert that exported source, use it as a `COMPILE APPLICATION` input with
the grammar/reader phases shown in `setup.build`. No `php_convert` executable
is supplied by this example.

## Current limits

This is a procedural subset translator, not a PHP implementation. `echo` uses
line-oriented `say`; translated variable names follow cREXX case-insensitive
binding. Runtime scalar values and function arguments use the documented
typed/text conversion subset. PHP references, copy-on-write compatibility,
complete key conversion, exact warnings and the full built-in library are not
provided. The design documents distinguish these limits from later extensions.

Phase 1 note: initialize a variable before an `if` or `while` block when the
variable is assigned inside that block and used afterward. This keeps the
generated cREXX binding in the outer scope; `php-sample-1.php` demonstrates
this with `$discount`.
