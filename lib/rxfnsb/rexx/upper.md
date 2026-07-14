# Level B `upper`

`upper` returns an uppercased copy of a string:

```rexx
upper(string = .string) = .string
```

Every cased character is converted by the VM's configured Unicode uppercase
mapping. Nonletters, uncased characters, and an empty string are unchanged.

```rexx
upper("MiXeD 123 !?") /* "MIXED 123 !?" */
upper("äöüé")         /* "ÄÖÜÉ" */
upper("")             /* "" */
```

The implementation does not modify the argument, and valid `.string` input has
no error branch. It uses a zero-copy exposed input binding followed by one
direct `strupper` instruction, creating only the result string.

This Level B helper uppercases the complete string. It does not accept start or
length arguments. `UPPER` is not a required Level C BIF in the repository's
Level C catalog; the common runtime's existing helper remains compatibility
surface only.
