## Level B `getenv`

`getenv` reads one variable from the running process environment:

```rexx
getenv(env_name=.string) = .string
```

The name is passed directly to the host platform. The result is the variable's
value, or the empty string when the variable is missing. A missing variable and
a variable whose value is empty are intentionally indistinguishable.

```rexx
home = getenv("HOME")
```

The input is not modified. The implementation executes the VM `getenv`
instruction once and has no Level B helper calls or intermediate string copy.
There is no Level C BIF with this name and no library signal contract; allocation
failure in the VM is fatal.

`lib/rxfnsb/tests_functional/ts_getenv.crexx` checks an injected value containing
spaces, repeatability, missing and empty names, and input non-mutation.
