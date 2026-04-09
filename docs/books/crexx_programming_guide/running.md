# Running \crexx{} on Linux, macOS and Windows

Linux and other Unix-like operating systems like Apple macOS behave in
an identical way when compiling, linking and running a \crexx{}
program. Windows can be used in almost exactly the same way, with some
caveats attached.


## Running cRexx entails compiling

All Rexx scripts you run with cRexx are compiled by \texttt{rxc} into
Rexx assembler code (.rxas) and then assembled into an .rxbin file,
which contains the Rexx bytecode for execution by the Rexx Virtual
Machine \texttt{rxvm}. This rxvm executable takes care of linking
separately compiled modules together and executing them, so one function
can find another.


## A first program


Let's say you have a Rexx exec you would like to run. To not have any
surprises, it is of the \emph{hello world} kind. We have a file called
`hellofirst.rexx`, containing:

```rexx <!--hellofirst.rexx-->
/* rexx */
options levelb
say 'hello cRexx world!'
```

<!--splice--hellofirst.rexx-->

When \crexx{} level C (for `Classic') is available, the `options levelb`
(on line 2) statement can be left out; for the moment, level B is all we
have, and the compiler will refuse to compile without it. This is in
part a reminder to the programmer that not all compatibility features
are implemented yet.
 
With all our cRexx executables on the PATH, we only need to do:

```bash <!--compile.sh-->
rxc hello
rxas hello
rxvm hello
```

to see `hello cRexx world!' on the console.

It might be a good idea to make a shell script to execute these three
programs in succession[^1]. But take into
account that this really is a very simple case, in which no built-in
functions are called. You can look into the generated rexx assembler
(hellofirst.rxas) file

<!--listasm--hellofirst.rxas-->

and you can see here that the compiler actually has generated a `say`
assembler instruction for the Rexx `say` instruction. But we did not yet call any
function.

[^1]: Don't worry, this is already delivered in the package as the crexx compiler driver.

## Using built-in functions

Most Rexx programs use the extremely well designed built-in functions.
Now with these functions written in Rexx, and not hidden in the compiler
somewhere, we must tell it to import those from the library where we put
them earlier during the build process. Let's say we want to add a
display of the current weekday to our hello program. This will now be:

```rexx <!--hellofunc.rexx-->
/* rexx */
options levelb
import rxfnsb
say 'hello cRexx world!'
say 'today is' date('w')
return 0
```

We can run this with `crexx hellofunc`. Its output is this

<!--splice--hellofunc.rexx-->

Never mind the import statement, which you will not need when \crexx{}
level C (for Classic) is available. But in level B, we need this, because we
need the flexibility it affords our plans for the future. Line 3, however, has the `import rxfnsb` statement,
which directs which version of the `date()` function we are going to use.

<!--listasm--hellofunc.rxas-->

The compiler will need to find the signature of the date()
function, so it can check if we call it in the correct way, with the
right parameters. As we see on line 35, this is the `date()` function it is going to look for.
In the standard distribution, it will find it in the right library as long as it
is in the same directory as the compiler executable, normally the `bin` directory.

## Using external functions

The built-in functions (BIF) are written in the \rexx{} language. External functions can be implemented in other languages, for example in a *plugin* written in C. For an explanation of the \crexx{} *Plugin Architecture* (rxpa), see page \pageref{pa---plugin-architecture}. Plugins can contain a multitude of functions[^methods].

An external function written in \crexx{}, of which the `.rxbin` file is found in the same source directory of the calling program, is found automatically. 

Regardless of the implementation in \rexx{} or a *plugin*, the mechanism works the same: an import statement enables the use of the external functions.

Because this is not a built-in function, we need to indicate the library where this plugin is to be found; for reasons of size, we cannot distribute an interpreter version which includes all available plugins linked in to it.

[^methods]: or methods, if we have written an object-oriented program.

## Building a standalone executable

To end this short tour of running cRexx, we are taking that last example
and will build a standalone executable out of it. If your neighbour,
family member or sports club runs the same OS as you do, they can now
run your compiled Rexx program from a USB stick, without ever installing
anything.

For this, we need the next set of commands, expressed as a Rexx exec:

This exec is delivered in the source tree, bin directory. At the moment
it can be run by classic rexx interpreters and NetRexx, it will soon be
runnable by \crexx{} itself. The exec works by compiling the Rexx program
specified (again without the .rexx file extension) to an .rxbin Rexx
bytecode file, which is then serialized to a C source file, containing
the cRexx Virtual Machine and the library rxbin files, by the rxcpack
command. It is a rather peculiar looking C source, but nevertheless it
will compile to a working executable, which is done by the last step in
the exec, here using the gcc compiler. And you will be able to run it
without the overhead of checking, compiling, tokenizing to bytecode and
linking, so it will be quite fast:

\begin{verbatim}
hello cRexx world!
today is Saturday
./hello  0.00s user 0.00s system 61% cpu 0.009 total
\end{verbatim}


