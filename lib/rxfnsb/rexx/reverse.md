# Level B `reverse`

`reverse` returns the Unicode codepoints of a string in reverse order:

```rexx
reverse(string = .string) = .string
```

The function reverses codepoints, not UTF-8 bytes. It does not combine or
reorder grapheme clusters: for example, a combining mark is an independent
codepoint. Empty and one-codepoint strings are returned unchanged, and the
source is read-only. Every valid `.string` is accepted, so there is no domain
error branch.

```rexx
reverse("abc")    /* "cba" */
reverse("aé日🙂") /* "🙂日éa" */
reverse("")       /* "" */
```

The implementation caches the character length and makes one reverse pass with
the VM `strchar` and `appendchar` operations. It makes no Level B helper call
and does not repeatedly copy the growing result with string concatenation.
