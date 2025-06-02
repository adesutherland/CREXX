# Command line arguments

Support for using command line arguments is built in the \rexx{} family of languages as fundamental and has its own statement, ```arg```, which picks up arguments to procedures as well as to the ```main()``` procedure, which is by convention the first procedure called by the operating system when a \crexx{} program is started.

As an example,

```rexx <!--args.rexx-->
options level dashcomments
import rxfnsb
arg fn = .string[]
-- when no commandline arguments found, display help
if fn[0]=0 then do
  call help
  exit
end
```

When there is program text before the first procedure, the ```main()``` procedure is automatically generated. In \crexx{} level B, the arguments to a procedure have a type, in the example this is a string array which is referenced in a variable of type ```.string[]```.
