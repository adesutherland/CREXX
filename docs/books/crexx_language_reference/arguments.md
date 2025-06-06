# Command line arguments

Support for using command line arguments is built in the \rexx{} family of languages as fundamental and has its own statement, ```arg```, which picks up arguments to procedures as well as to the ```main()``` procedure, which is by convention the first procedure called by the operating system when a \crexx{} program is started.

As an example,

```rexx <!--args.rexx-->
options levelb dashcomments
import rxfnsb
arg fn = .string[]
-- when no commandline arguments found, display help
if fn[0]=0 then do
  call help
  exit
end

help: procedure
say 'this program needs arguments'
```

<!--splice--args.rexx-->

When there is program text before the first procedure, the ```main()``` procedure is automatically generated. In \crexx{} level B, the arguments to a procedure have a type, in the example this is a string array which is referenced in a variable of type ```.string[]```.

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

