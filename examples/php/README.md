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

The exported source can then be converted by the standalone cREXX driver:

```powershell
php_convert.exe php_embedded_source.php php_embedded_converted --run
```

The driver expects `php_reader-linked.rxbin` in the current directory. Set
`CREXX_PHP_READER` when the reader is elsewhere. Without `--run`, it stops
after generating `php_embedded_converted.crexx` and
`php_embedded_converted.rxbin`.

The reader expects the generated `phpgrammar` module beside it. Its conceptual
invocation is:

```powershell
php_reader.exe migration.php migration.crexx
```

The output file is replaced on each conversion, so rerunning the reader does
not append a second translation.

Generated grammar and reader binaries belong in the build directory and are
not source assets.

Phase 1 note: initialize a variable before an `if` or `while` block when the
variable is assigned inside that block and used afterward. This keeps the
generated cREXX binding in the outer scope; `php-sample-1.php` demonstrates
this with `$discount`.
