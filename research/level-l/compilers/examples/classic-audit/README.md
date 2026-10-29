# From Classic Rexx source to two useful checks

Hi Tom,

Can a division run without a valid zero check? Can a loop restart without
updating its counter? Your point about needs hidden inside a routine suggested
these two audits.

I tried these in an experimental cREXX auditor. It imports Classic Rexx through
Level L's compiler frontend, then applies two demo rules to the resulting
syntax tree and compiler graph.

Start with [average.rexx](average.rexx):

```rexx
parse arg total, count             /* Read total and count. */
if count = 0 then return 'no items' /* Handle zero before division. */
return total / count              /* Calculate the average. */
```

In [audit.crexx](audit.crexx), these calls import the source:

```rexx
source = readbinary(argv[1])                           /* Read the source file. */
front = .llcfrontend..request(source,argv[2],filename,100000) /* Parse Classic Rexx. */
tree = front.ast(); graph = front.graph()              /* Get its tree and compiler operations. */
index = .llirgraph..index(reference graph)             /* Index routes and value definitions. */
```

The **syntax tree** records source constructs. The **intermediate
representation (IR)** groups executable operations into blocks, connected by
possible execution routes.

The demo's `divisions()` rule follows each divisor to the operation that
produced it:

```rexx
id = index.definition(value) /* Find this value's defining operation. */
op = graph.operation(id)     /* Inspect its operands. */
```

Here it finds a read of `COUNT`, then looks for a comparison with literal `0`.
Level L's **dominance** query asks whether every route to the division passes
the comparison's nonzero destination:

```rexx
successor = terminal.successor(nonzero)                          /* Select the nonzero destination. */
if index.dominates(successor,index.opblock(division)) = 0 then iterate /* Reject a bypassable check. */
write = changed(guardread,divisor,symbol)                         /* Look for a replacement value. */
```

The rule also checks that only the intended branch enters that destination.
Its `changed()` helper follows the original graph to find any assignment or
argument parse that replaces the checked value on a route to its use,
including later loop iterations. Coverage and an unchanged value are both required:

```text
average.rexx:
  Line 3: division by COUNT is protected by the zero check on line 2.
```

Move the check below the division, as in
[average-late.rexx](average-late.rexx):

```text
average-late.rexx:
  Line 2: no covering zero check found for division by COUNT.
```

In [average-loop.rexx](average-loop.rexx), the check precedes the loop, but
the loop subtracts one from `COUNT` after dividing:

```text
  Line 4: COUNT is changed on line 5 after its zero check on line 2.
```

With `total=2` and `count=1`, the second division uses zero. Moving the check
inside the loop, before division, protects each use.

Now try [readings.rexx](readings.rexx), which averages usable numbers:

```rexx
parse arg readings                    /* Read a list. */
remaining = words(readings)            /* Count its words. */
accepted = 0                          /* Count usable readings. */
total = 0                             /* Start their sum. */
do while remaining > 0                /* Work backwards. */
  value = word(readings, remaining)    /* Read this item. */
  remaining = remaining - 1           /* Advance before restarting. */
  if datatype(value, 'N') = 0 then iterate /* Skip nonnumbers. */
  if value < 0 then iterate           /* Skip negatives. */
  total = total + value               /* Add this reading. */
  accepted = accepted + 1             /* Count it. */
end                                  /* Continue. */
if accepted = 0 then return 'no valid readings' /* Handle an empty result. */
return total / accepted               /* Calculate the average. */
```

The tree exposes loops through named fields:

```rexx
walk = tree.walk('parents first')       /* Visit source constructs. */
do while walk.next()                   /* Advance the walk. */
  node = walk.node()                   /* Get this construct. */
  if node.kind() <> 'CDo' then iterate  /* Select Classic DO. */
  headernode = node.child('header')     /* Read its header. */
end                                   /* Continue searching. */
```

The demo's `loops()` rule recognises `counter > 0` and a single
`counter = counter - 1`, using literal `0` and `1` exactly. Level L finds loops
and returning edges; dominance checks that each returning route passes the
decrement:

```rexx
regions = index.loops()                                     /* Find compiler-graph loops. */
if regions.backedge(source,header) = 0 then iterate          /* Select a returning edge. */
if index.dominates(index.opblock(update),source) = 0 then every = 0 /* Require the update. */
```

```text
readings.rexx:
  Line 14: division by ACCEPTED is protected by the zero check on line 13.
  Loop on line 5: every route back to the test executes REMAINING = REMAINING - 1 (line 7).
```

Move the decrement below both `ITERATE` statements and remove the final zero
check. [readings-broken.rexx](readings-broken.rexx) gives:

```text
readings-broken.rexx:
  Line 13: no covering zero check found for division by ACCEPTED.
  Loop on line 5: a route back to the test skips the decrement of REMAINING.
    ITERATE on line 7 bypasses the decrement.
```

An empty list reaches `0/0`; an invalid reading can repeat forever. The loop
rule establishes execution of the decrement on every returning route. Numerical
termination also needs an argument about progress at the chosen precision.

These all-route checks complement SLAM's per-input certificates and help
explore the all-input questions from our discussion.

Use an existing [cREXX Classic research build](https://github.com/adesutherland/CREXX/blob/develop/research/level-l/compilers/README.md#build-and-install),
then run these commands from this example directory:

```sh
./run.sh /path/to/CREXX/cmake-build-debug                 # Build and analyse all eight examples.
./run.sh /path/to/CREXX/cmake-build-debug my-program.rexx # Analyse your Classic Rexx file.
./run.sh /path/to/CREXX/cmake-build-debug --check          # Run the 26 controls.
```

The [audit.crexx](audit.crexx) profile admits standalone scalar code and the
pure `WORDS`, `WORD` and `DATATYPE` built-ins. It reports unsupported constructs,
including dynamic code, handlers and user functions, at their source lines.
[check.sh](check.sh) includes branch and repeated-iteration controls.
