# Global Procedures

## Exposing Global Procedures

A procedure defined in a module file can be made available to other modules (provided it is in an imported namespace) by including the procedure in the namespace instruction.

```rexx <!--exposeex.crexx-->
namespace mynamespace expose myfunc1 myfunc2
```

## Importing Global Procedures

When compiling a program, the compiler looks for procedures that are called but not defined in the current module. It searches in various locations in the following order:

1. The primary source root: source files in the directory of the source being
   compiled, or in the working location selected with `-l` when the source name
   has no directory.
2. Additional source roots specified with `rxc -s`: source files.
3. Binary roots specified with `rxc -i`: `.rxbin` files, optional `.rxas` files
   when `--import-rxas` is enabled, and cRexx native function libraries
   (plugins).
4. The directory where the compiler executable (`rxc`) is located: deployed
   `.rxbin` files, optional `.rxas` files when `--import-rxas` is enabled, and
   plugins.

The source root is not automatically searched for `.rxbin` files. Pass `-i .`
or `-i build-dir` when a local binary module is an intended compile-time
dependency. This separation avoids stale generated `.rxbin` files beside source
files being imported by accident.

Source roots search `.crexx`, `.crx`, and `.rexx`. If the initial source file
uses another extension, such as `.the`, that extension is searched for this
compile too. Files without an explicit `options level...` default to Level G forcRexxcrx`, and arbitrary initial extensions, and Level C for `.rexx`.

The compiler only uses external procedures that are in the same namespace as the module being compiled or for namespaces listed in the "import" instruction. 

```rexx <!--importex.crexx-->
import rxfnsb mynamespace anothernamespace
```

Metadata encoded with the procedures allows the compiler to determine the type and argument types of imported procedures.

The cRexx level b standard library namespace is `rxfnsb`, to access these procedures programs should import this namespace.

```rexx <!--import2.crexx-->
import rxfnsb
```

Without the `import` statement for the `rxfnsb` namespace, the level B built-in functions cannot be found. One could say that the built-in functions are not as built-in as they are in Classic Rexx. This is different in level C, the compatibility level.
