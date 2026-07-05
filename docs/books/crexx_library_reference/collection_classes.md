# Collection Classes

The collection classes provide an object-oriented idiom for handling (adding, deleting, sorting, searching) of data. All are part of the data namespace. They implement interfaces like `.Iterable`, `.Iterator`, `.List`, `.Map` and `.Tree`.

## Strings and collections of strings

The `.rexx` class offers functionality that is on par with the Object Rexx and NetRexx object-oriented notation for strings. It returns `.rexx` objects so calls can be chained.

The `List`, `Set`, `Map` and `Tree` interfaces have class implementations for `.string` type collections.

## Collections of Objects

## Notes on performance

The `StringTreeMap` class is based on an AVL[^avl] Tree for optimal performance. This is a balanced binary tree with guaranteed O$log N$ performance for all operations. This class is written in cRexx and its performance has been benchmarked against a *red-black tree* implementation in C, which is what most class libraries use. Its performance is identical to that native implementation.

[^avl]: <!--cite-->[sedgewick2003]
