## Building and Running cRexx

This paragraph aims to show all you need to know about how to build cRexx from scratch, and then run it. It will show what you need, what to do and when it is build, how to make working programs with it.

### Requirements
Currently cRexx is known to build on Windows, macOS and Linux[^1]. You need one of those and:

[^1]: There is a separate instruction for VM/370 (and later) mainframe operating systems.

Tool | Function 
-----|-----
Git  | source code version management
CMake| build tool
Rexx | used during build process (brexx, ooRexx, Regina will all do)
C compiler | gcc, clang (install g++ as some C++ elements are used) 
Bison | parser generator


and either

Tool | Function
-----|-----
Make| conventional build tool
Ninja| fast build tool

#### Platform specific info
On Linux and macOS, this instruction is identical. For macOS, Xcode batch tools need to be installed, which will provide you with git, make and the compiler. Brew will give easy access to regina-rexx[^2] and Ninja-build.

[^2]: ooRexx and brexx will also work, one of those needs to be on the path.
For Linux, you will need to install git (which will be there on most distributions), cmake and gcc or clang.

On Windows, you will need a compatibility layer like [msys](https://www.msys2.org) - installing this has the additional advantage of easy access to git, gcc, cmake and the rest of the necessary tools. On more modern Windows, the WSL[^3}] and Ubuntu is not a bad choice.

[^3]: WSL: Windows subsystem for Linux.

### Process
Here it is assumed that all tools are installed and working, and available on your PATH environment variable.

Choose or make a suitably named directory on your system to contain the source code. Note that the cRexx source is kept in a different directory on you system than where it is built in, or will run from. Now run this command:
```
git clone https://github.com/adesutherland/CREXX.git
```
This will give you a CREXX subdirectory in the current directory, containing the source of cRexx and its dependencies. This is the 'develop' branch, which is the one you would normally want to use. All of these are written in the C99 version of the C programming language, which should be widely supported by C compilers on most platforms.

Make a new subdirectory in the current directory (not in CREXX, but in the one that contains it), like `crexx-build'.
```
mkdir crexx-build
```
and cd into that directory. Now issue the following command (we assume that you installed ninja, otherwise subsitute 'make' for the two instances of 'ninja'):
```
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../CREXX && ninja && ctest --output-on-failure
```
This will do a lot of things. In fact, if all goes well, you will have a built and tested cRexx system.
<!-- ```mermaid -->
<!-- flowchart LR -->
<!--    source -- cmake -\-> build.ninja -- ninja -\-> executables -- ctest -\-> tests -->
<!-- ``` -->
### Explaining the build process
Let's zoom in a little on what we did. The first step is to tell CMake to validate your system environment and generate a build script (and a test script) for the chosen build tool. Cmake will read the file CmakeLists.txt and validate that your system can do what it asks it to do. This can yield error messages, for example if the C compiler lacks certain functions or header files. (When that happens, open an [issue](https://github.com/adesutherland/CREXX/issues) and someone will have a look at it - or peruse [stack overflow](https://stackoverflow.com) which is what we probably also will do).

After CMake has successfully validated the build environment, it will generate a build script (a Makefile in the case of Make and a build.ninja file in the case of Ninja). This is specified after the '-G' flag. The '-DCMAKE_BUILD_TYPE=Release' flag makes sure we do an optimized build, which means we specify an '-o3' flag to the C compiler, which then will spend some time optimizing the executable modules, which makes them run faster (they do!). The alternative is a 'debug' build which will yield slower executables, but with more debugging information in them.

The two ampersands (&&) mean we do the next part only if the previous step was successful. This is a 'ninja' statement, which will build everything in the build.ninja specification file. These are a lot of parts, and the good news is, when they are built once, only the changed source will be built, which will be fast.

After this, the generated test suite is run with the 'ctest' command. This knows what to do, and will show you successes and failures. If what you checked out if git is not a released version, there is a small change that some test cases fail, but generally these should indicate success.

### Use of Rexx to build cRexx
Rexx code is used twice during the build process. The first time is in an early stage: as the Rexx VM 'machine code' instructions are generated from a file, a Rexx script needs to work on these to generate two C sources. (The Rexx engine for this needs to be in working order on the building system - this is one of the things CMake checks, and it can flag down the build if it cannot locate a working 'rexx' executable).

The second time Rexx is used, it is already our working cRexx instance: the library of built-in functions is written in Rexx (and Rexx Assembler) and needs to be compiled (carefully observing the dependencies on other Rexx built-in functions) before it is added to the library and the cRexx executables.

### What do we have after a successful build process
When all went well, we have a set of native executables for the platform we built cRexx on. These are

Name | Function
-----|------------
rxc  | cRexx compiler
rxas | cRexx assembler
rxdas| cRexx disassembler
rxvm | cRexx virtual machine
rxbvm| cRexx virtual machine, non-threaded version
rxvme| cRexx virtual machine with linked-in Rexx library
rxdb | cRexx debugger
rxcpack| cRexx C-generator for native executables

You read that right, cRexx already can compile your Rexx script into an executable file, that can be run standalone, for example, on a computer that has no cRexx and/or C compiler installed.

Another salient fact from the above table is that 'rxdb', the cRexx debugger, is entirely written in Rexx and compiled and packaged into an executable file.

The executables in the above table need to be on the PATH environment variable. These are to be found in the compiler, assembler, disassembler, debugger and cpacker directories of the crexx-build directory we created earlier. It is up to you to add all these separately to the PATH environment, or to just collect them all into one directory that is already on the PATH.

## Running cRexx
All Rexx scripts you run with cRexx are compiled by 'rxc' into Rexx assembler code (.rxas) and then assembled into an .rxbin file, which contains the Rexx bytecode for execution by the Rexx Virtual Machine 'rxvm'. This rxvm executable takes care of linking separately compiled modules together and executing them, so one function can find another.

### An easy test - Hello World
Let's say you have a Rexx exec you would like to run. To not have any surprises, it is of the 'hello world' kind.
We have a file called hello.rexx, containing:
```
/* rexx */
options levelb
say 'hello cRexx world!'
return 0
```
When cRexx level 'C' (for 'Classic') is available, the 'options levelb' (on line 2) statement can be left out; for the moment, level B is all we have, and the compiler will refuse to compile without it.

With all our cRexx executables on the PATH, we only need to do:
```
rxc hello
rxas hello
rxvm hello
```
to see 'hello cRexx world!' on the console.
<!-- ```mermaid -->
<!-- flowchart LR -->
<!--    hello.rexx -- rxc hello -\-> hello.rxas -- rxas hello -\-> hello.rxbin -->
<!-- ``` -->


It might be a good idea to make a shell script to execute these three programs in succession, and perhaps call it 'crexx'. But take into account that this really is a very simple case, in which no built-in functions are called. You can look into the generated rexx assembler (hello.rxas) file:
```
➜  crexx git:(master) ✗ cat hello.rxas
/*
 * cREXX COMPILER VERSION : cREXX F0043
 * SOURCE                 : hello
 * BUILT                  : 2022-12-03 22:27:52
 */

.srcfile="hello"
.globals=0

main() .locals=1 .expose=hello.main
   .meta "hello.main"="b" ".int" main() "" ""
   .src 3:1="say 'hello cRexx world!'"
   say "hello cRexx world!"
   .src 4:1="return 0"
   ret 0
```
and you can see here that the compiler actually has generated a 'say' assembler instruction for the Rexx 'say' instruction. (Assembler became a whole lot easier with Rexx assembler.) But we did not yet call any function.

### Using built-in functions
Most Rexx programs use the extremely well designed built-in functions. Now with these functions written in Rexx, and not hidden in the compiler somewhere, we must tell it to import those from the library where we put them earlier during the build process. Let's say we want to add a display of the current weekday to our hello program. This will now be:
```
/* rexx */
options levelb
import rxfnsb
say 'hello cRexx world!'
say 'today is' date('w')
return 0
```
Never mind the import statement, which you will not need when cRexx 'Classic' level C is available. But in level B, we need this, because we need the flexibility it affords our plans for the future.

We must tell the compiler where to find the signature of the date() function, so it can check if we call it in the correct way, with the right parameters. This is done with the -i switch, which points to the directory containing the library - which is called 'library', by the way.

```
rxc -i ~/crexx-build/lib/rxfns hello
```
The assembler runs unchanged, because it trusts the compiler to have checked if the called function really sits in that library, and has the right parameters - the right code to call it has been generated.
```
rxas hello
```
To run it, we can employ the 'rxvme' executable - this one is extended with linked-in versions of all the functions in the library:
```
rxvme hello
```
which yields:
```
hello cRexx world!
today is Saturday
```

### Building a standalone executable
To end this short tour of building and running cRexx, we are taking that last example and will build a standalone executable out of it. If your neighbor, family member or sports club runs the same OS as you, they can now run your rexx program from a USB stick, without ever installing anything.

For this, we need the next set of commands, expressed as a Rexx exec:

```
/* rexx compile a rexx exec to a native executable */
/* Classic Rexx and NetRexx compatible             */
crexx_home='~/crexx-build'
if arg='' then do
  say 'exec name expected.'
  exit 99
end
parse arg execName'.'extension
if extension<>'' then say 'filename extension ignored.'
'rxc  -i' crexx_home'/lib/rxfns' execName
'rxas' execName
'rxcpack' execName crexx_home'/lib/rxfns/library'
'gcc -o' execName,
'-lrxvml -lmachine -lavl_tree -lplatform -lm -L',
crexx_home'/interpreter -L'crexx_home'/machine -L',
crexx_home'/avl_tree -L'crexx_home'/platform'  execName'.c'
```
This exec is delivered in the source tree, bin directory. At the moment it can be run by classic rexx interpreters and NetRexx, it will soon be runnable by cRexx itself.
The exec works by compiling the Rexx program specified (again without the .rexx file extension) to an .rxbin Rexx bytecode file, which is then serialized to a C source file, containing the cRexx Virtual Machine and the library rxbin files, by the rxcpack command. It is a rather peculiar looking C source, but nevertheless it will compile to a working executable, which is done by the last step in the exec, here using the gcc compiler. And you will be able to run it without the overhead of checking, compiling, tokenizing to bytecode and linking, so it will be quite fast:
```
➜  crexx git:(master) ✗ time ./hello
hello cRexx world!
today is Saturday
./hello  0.00s user 0.00s system 61% cpu 0.009 total
```
