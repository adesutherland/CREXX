# Command line arguments

Support for using command line arguments is built in the Rexx family of languages as fundamental and has its own statement, ```arg```, which picks up arguments to procedures as well as to the ```main()``` procedure, which is by convention the first procedure called by the operating system when a cRexx program is started.

As an example,

```rexx <!--args.rexx-->
options levelb
import rxfnsb
arg fn = .string[]
/* when no commandline arguments found, display help */
if fn[0]=0 then do
  call help
  exit
end

help: procedure
say 'this program needs arguments'
```

When there is program text before the first procedure, the ```main()``` procedure is automatically generated. In cRexx level B, the arguments to a procedure have a type, in the example this is a string array which is referenced in a variable of type ```.string[]```.

After this assignment by ```arg fn = .string[]``` (in line 3) the number of command line arguments to the program is contained in ```fn.0```, which can also be addressed as ```fn[0]```. We can now use these arguments in the program, for example by looping over the elements of the string array.

```rexx <!--loopover.rexx-->
/* loop through arguments and find options and program files */
do i=1 to fn.0
  if left(fn.i,1)<>'-' then
    do /* it is not a flag but a filename */
      filename=fn.i
      filenames=strip(filenames) strip(filename)
    end
  else
    do
      if fn.i = '-noexec' then execute=0
[...]
```

These two fragments illustrate how to handle command line arguments to a program; when there are no arguments, ```fn.0``` is 0 and we call a ```help``` function to explain about the usage of this program. If there are arguments, we loop over them and decide whether they are options (these start with a dash) or files (if they don't).

## Procedure arguments and expose

Named procedures use the same `ARG` statement to bind call arguments. Level B
code normally gives each argument a type:

```rexx
worker: procedure = .int
  arg value = .int, name = .string
  return value
```

Plain `arg name = type` parameters are by value. If the procedure must update
the caller's variable, declare that parameter with `expose`:

```rexx
bump: procedure = .void
  arg expose value = .int
  value = value + 1
  return
```

The `procedure expose` clause is different: it exposes module-global storage to
the procedure, while `arg expose` exposes a call argument by reference.

Optional arguments can be tested with the `?name` form. A final `... = type`
tail accepts a variable number of arguments; those values are available through
the pseudo array `arg[]`/`arg.0` and the compatibility `arg()` operator. See
`statements.md` for the complete procedure, argument, ellipsis, and `arg()`
rules.
