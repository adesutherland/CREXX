# Level B `lower`

`lower` returns a lowercased copy of a string:

```rexx
lower(string = .string) = .string
```

Every cased character is converted by the VM's configured Unicode lowercase
mapping. Nonletters, uncased characters, and an empty string are unchanged.

```rexx
lower("MiXeD 123 !?") /* "mixed 123 !?" */
lower("ÄÖÜÉ")         /* "äöüé" */
lower("")             /* "" */
```

The argument is read-only and valid `.string` input has no error branch. The
implementation is one direct `strlower` instruction and creates only the result
string.

This Level B helper lowercases the complete string. It does not accept start or
length arguments. `LOWER` is not a required Level C BIF in the repository's
Level C catalog; the common runtime's existing helper remains compatibility
surface only.
