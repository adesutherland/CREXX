## Level B `upper`

`upper` returns an uppercased copy of a string:

```rexx
upper(string = .string) = .string
```

Every character covered by the VM's deliberately limited, locale-independent
simple uppercase table is converted. Other characters and an empty string are
unchanged; this is not full Unicode case folding.

```rexx
upper("MiXeD 123 !?") /* "MIXED 123 !?" */
upper("äöüé")         /* "ÄÖÜÉ" */
upper("")             /* "" */
```

U+0000 is an ordinary codepoint for this bounded operation; it does not stop
conversion of later text.

The implementation does not modify the argument, and valid `.string` input has
no error branch. Its ordinary value argument is proved read-only from the
classified `strupper` operand effects, so the compiler can share the incoming
value without a defensive copy. The typed result is completely defined by one
direct `strupper` instruction, so an optimized build needs no prior result
value.

This Level B helper uppercases the complete string. It does not accept start or
length arguments. `UPPER` is not a required Level C BIF in the repository's
Level C catalog; the common runtime's existing helper remains compatibility
surface only.
