# From Rexx code to a useful warning

Tom suggested SLAM's square-root example. We converted its routine declaration
and argument handling to Classic Rexx, then gave the code to Level L.

Run the cREXX walkthrough with an existing cREXX research build:

```sh
./run.sh /path/to/CREXX/cmake-build-debug  # Build and run the cREXX walkthrough.
```

## 1. Import the code

The Classic frontend uses a Level L grammar to turn the source into an
**abstract syntax tree**: nodes for assignments, expressions, branches and loops.

```rexx
front = .llcfrontend..request(source,runtime,'fdsqrt-new.rexx') /* Create a Classic Rexx parsing request. */
tree = front.ast()                                          /* Get the parsed source tree. */
```

## 2. Walk the tree

`node.each()` visits a node's children. The walkthrough's `showtree()` helper
uses it to print the loop:

```rexx
children = node.each()                           /* Prepare to visit this node's children. */
do while children.next()                         /* Advance to each child until none remain. */
  call showtree(children.node(),depth + visible)  /* Print the child and visit its children. */
end                                             /* Finish this node's child loop. */
```

```text
Loop forever
  Set next = (guess+(x/guess))/2
  If next>=guess
    Leave loop
  Set guess = next
return guess
```

We can see the calculation, the exit test and the update as separate nodes.

## 3. Follow the execution routes

We translate the tree into **execution roles**: assignment, branch, loop and
return. Level L connects these into a **flow graph**, which follows the routes
through the code.

```rexx
flowtree = sqrtflow..lower(reference tree)       /* Translate syntax nodes into execution roles. */
flow = sqrtflow..execution(reference flowtree)   /* Connect those roles into a flow graph. */
call branches(reference flow,guard)             /* Print the exit test's true and false routes. */
```

```text
Test: next>=guess
  true -> leave
  false -> guess=next
```

The true route leaves before updating `guess`. The routine returns the previous
estimate on that route.

## 4. Prove an assignment, then break it

**Definite assignment** checks that every route to a read passes an assignment.
`X` is the routine's argument; we supply it as assigned at entry.

```rexx
facts = flow.definiteassignment('X')           /* Check assignments, with argument X assigned at entry. */
say yesno(facts.before(guard).assigned('NEXT')) /* Report whether every route to the test assigns NEXT. */
```

```text
yes
```

Now change one line:

```rexx
if x>1 then next=(guess+(x/guess))/2 /* Assign NEXT only when X is greater than 1. */
```

Run the same analysis:

```text
NEXT assigned on every path to the test: no
Line 9: NEXT may be read before it is assigned.
```

`badfacts.witness(1)` supplies the route: take the false branch of `if x>1`,
then reach the comparison without assigning `NEXT`. In Rexx, that leaves the
symbol text `NEXT` where the calculation expects a number.

## 5. Go down to compiler operations

The frontend also produces **intermediate representation**, or IR: simple
operations grouped into **blocks**, each a straight sequence. Level L supplies
standard graph algorithms for the jumps between blocks.

```rexx
graph = front.graph()                     /* Get the compiler's operation graph. */
index = .llirgraph..index(reference graph) /* Index blocks, jumps and value uses. */
loops = index.loops()                     /* Find cycles and loop regions. */
value = op.result(1)                      /* Get the selected division's first result. */
useid = index.use(value.id(),1)           /* Find the first operation that consumes it. */
```

For the division operation, following its result gives:

```text
Loop analysis found 1 loop region.
Value path: x/guess -> addition
Every path to that addition passes the division block: yes
```

**Strongly connected component analysis** finds blocks that can loop back to
one another. **Dominator analysis** establishes that every path to one block
passes through another; the walkthrough queries it with `index.dominates()`.

SLAM can use these structured fragments, execution routes and explained
warnings when selecting and comparing code.

The runnable code is in [journey.crexx](journey.crexx); the tree translation
and flow mappings are in [flowview.crexx](flowview.crexx).
The [original FDSqrt](https://github.com/animatedads/alchemy/blob/main/packages/fd_door_micro_motion_v0.2-dev8/src/FDDoorMicroMotion.cls)
is credited to ChatGPT. SLAM's design is Tom Dyer's, with code by Claude.
