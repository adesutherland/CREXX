## Level B `sourceinfo`

`sourceinfo()` returns the source context of the root program:

```rexx
sourceinfo() = .string
```

The result contains three space-separated fields:

```text
system invocation-mode source-file-name
```

`system` is the host platform name, `invocation-mode` is currently `COMMAND`
or `FUNCTION`, and `source-file-name` is taken from the program's source
metadata. `PARSE SOURCE` will use this function as its source expression.
