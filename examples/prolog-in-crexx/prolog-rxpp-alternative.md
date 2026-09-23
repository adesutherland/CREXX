# Defining Prolog Data with RXPP Blocks

Prolog data can be added to a cREXX program in the traditional way by
constructing a cREXX array and passing it explicitly to
`prolog..PrologSourceAppendArray`:

```rexx
person = .array
person[1] = 'person(alice).'
person[2] = 'person(bob).'

call prolog..PrologSourceAppendArray person
```

For larger Prolog programs, RXPP also supports a more readable block
notation:

```rexx
##relation person
  person(alice).
  person(bob).
##end
```

The contents of the block are collected as Prolog source in a cREXX array.
The array can then be appended using the existing Prolog interface:

```rexx
call prolog..PrologSourceAppendArray person
```

Relations and rules can therefore remain recognisably Prolog instead of
being assembled line by line in cREXX:

```rexx
##rule colleague
  colleague(X,Y) :-
    works_in(X,D),
    works_in(Y,D),
    X \= Y.
##end
```

The block notation does not introduce a second Prolog syntax. It is only a
convenient RXPP representation of the same source data. The traditional
array-based interface remains available, and both approaches can be used in
the same program.

The optional clause following the block name, such as `WITH`, `INTO`, or
`FOR`, is currently illustrative only. It helps make the intended handling
of the block more apparent, but has no operational meaning at present.

After all relations and rules have been collected, the complete Prolog
source is installed in the usual way:

```rexx
call prolog..PrologConsultExecute
```