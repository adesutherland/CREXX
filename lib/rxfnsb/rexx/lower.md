# Level B `lower`

`lower` returns a lowercased copy of a string:

```rexx
lower(string = .string) = .string
```

Every character covered by the VM's deliberately limited, locale-independent
simple lowercase table is converted. Other characters and an empty string are
unchanged; this is not full Unicode case folding.

```rexx
lower("MiXeD 123 !?") /* "mixed 123 !?" */
lower("ÄÖÜÉ")         /* "äöüé" */
lower("")             /* "" */
```

U+0000 is an ordinary codepoint for this bounded operation; it does not stop
conversion of later text.

The argument is read-only and valid `.string` input has no error branch. The
implementation is one direct `strlower` instruction and creates only the result
string.

This Level B helper lowercases the complete string. It does not accept start or
length arguments. `LOWER` is not a required Level C BIF in the repository's
Level C catalog; the common runtime's existing helper remains compatibility
surface only.
