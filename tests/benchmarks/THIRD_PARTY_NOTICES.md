# Third-party notices

## Are We Fast Yet? / SOM-derived benchmarks

The Sieve, Permute, Towers, Bounce, Storage, List, Richards, Queens and
DeltaBlue ports are based on the SOM/Java benchmarks distributed by the Are We
Fast Yet? project.
Copyright (c)
2001–2021, see the upstream project authors. The Mandelbrot port follows the
Are We Fast Yet? version derived from the Computer Language Benchmarks Game.
The NBody port follows the Are We Fast Yet? version of the Computer Language
Benchmarks Game workload. The expanded ports were reviewed against upstream commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`.

The historical deterministic JSON fixture used by `json_parser.crexx`,
`json_parse.crexx` and `json_query.crexx` has a smaller RAP-style request/data
shape. It is a capability probe and is not represented as a byte-for-byte copy
of the upstream input or full upstream benchmark.

`fixtures/awfy_json_rap_minified.json` is different: it is the exact minified
25,820-byte RAP payload embedded by the upstream Full Json workload at commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`, retained for the separately named
`awfy_json.crexx` port. Its SHA-256 is
`8f84f5fdc609a6d7179089249212a39588030852719d951db2d178820b70a7d8`.
The cREXX port retains the upstream observable verification contract but uses
the supported indexed `.jsondocument` representation and is labelled as an
adaptation.

`awfy_deltablue.crexx` derives its chain/projection algorithms and deterministic
assertions from the same pinned Java/SOM sources. Its stable integer handles,
planner-owned typed arrays and tagged constraint representation are disclosed
Level B adaptations for the language's value-copy object semantics.

`awfy_cd.crexx` derives its 200-frame simulator, collision detector, voxel
reduction and red/black-tree algorithms from the CD sources at the same pinned
commit. Copyright (c) 2001-2010 Purdue University and Copyright (C) 2015 Apple
Inc.; the complete upstream revised-BSD notice is retained in the source file.
The indexed nodes/occurrences and `rxmath` boundary are disclosed Level B
adaptations.

`awfy_havlak.crexx` derives its CFG builder, Havlak/Tarjan loop recognizer,
union-find and loop-structure graph from the Google Havlak sources at the same
pinned commit. Copyright 2011 Google Inc. The port retains the upstream Apache
License 2.0 header. Stable integer handles and typed arrays are disclosed Level
B adaptations for object identity and collection storage.

MIT license used by the SOM-derived sources:

> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the “Software”), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in
> all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

The upstream project and citation metadata are at
<https://github.com/smarr/are-we-fast-yet>.

The complete shared MIT text is in `LICENSE-SOM-MIT.txt`. The upstream
Benchmarks Game license text is preserved in `LICENSE-AWFY.md`.

## Google Havlak benchmark

The Havlak-derived source is licensed under the Apache License, Version 2.0.
The license terms are available at
<https://www.apache.org/licenses/LICENSE-2.0>. The required copyright and
license notice is retained at the head of `awfy_havlak.crexx`.

The Computer Language Benchmarks Game material is distributed under the
Revised BSD license. Copyright 2008–2012 Isaac Gouy; the Are We Fast Yet?
Mandelbrot source additionally identifies Brent Fulgham (2004–2013) and its
individual contributors. Redistribution and use in source and binary forms,
with or without modification, are permitted provided the copyright notice,
conditions, and disclaimer are retained; contributor and project names may not
be used to endorse derived products without permission. The material is
provided without warranty and without liability for damages.

## RexxCPS

`rexxcps_levelb.crexx` is derived from the ooRexx RexxCPS 2.2 sample:

- Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.
- Copyright (c) 2005, 2006 Rexx Language Association. All rights reserved.
- Copyright (c) 1981, 2025 Mike Cowlishaw. All rights reserved.

The upstream file states that it and the accompanying materials are available
under the Common Public License v1.0, and also grants redistribution and use in
source and binary forms, with or without modification, provided that the
copyright notice, conditions, and disclaimer are retained. Neither the Rexx
Language Association nor contributor names may endorse derived products
without written permission. The program is provided without warranty and
without liability for direct, indirect, incidental, special, exemplary, or
consequential damages.

The complete upstream header is retained at the start of the port. The
unmodified source is available from the [ooRexx source
repository](https://sourceforge.net/p/oorexx/code-0/HEAD/tree/main/trunk/samples/rexxcps.rex?format=raw),
and the full CPL 1.0 is included as `LICENSE-REXXCPS-CPL-1.0.txt`.
