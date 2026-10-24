# Phase 1 PHP-to-cREXX converter

This example is the first source-to-source migration slice described in
[`design-php.md`](design-php.md). The Level L grammar defines the supported PHP
syntax; [`php_reader.crexx`](php_reader.crexx) walks the generated parse tree
and writes readable cREXX source.

[`migration.php`](migration.php) exercises scalar variables, arithmetic,
string concatenation, `echo`, `if/else`, and `while`.

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
