# Global Procedures

## Exposing Global Procedures

A procedure defined in a module file can be made available to other modules (provided it is in an imported namespace) by including the procedure in the namespace instruction.

```rexx
namespace mynamespace expose myfunc1 myfunc2
```

## Importing Global Procedures

When compiling a program, the compiler looks for procedures that are called but not defined in the current module. It searches in various locations in the following order:

1. The primary source root: `.rexx` files in the directory of the source being
   compiled, or in the working location selected with `-l` when the source name
   has no directory.
2. Additional source roots specified with `rxc -s`: `.rexx` files.
3. Binary roots specified with `rxc -i`: `.rxbin` files, optional `.rxas` files
   when `--import-rxas` is enabled, and CREXX/C native function libraries
   (plugins).
4. The directory where the compiler executable (`rxc`) is located: deployed
   `.rxbin` files, optional `.rxas` files when `--import-rxas` is enabled, and
   plugins.

The source root is not automatically searched for `.rxbin` files. Pass `-i .`
or `-i build-dir` when a local binary module is an intended compile-time
dependency. This separation avoids stale generated `.rxbin` files beside source
files being imported by accident.

The compiler only uses external procedures that are in the same namespace as the module being compiled or for namespaces listed in the "import" instruction. 

```rexx
import rxfnsb mynamespace anothernamespace
```

Metadata encoded with the procedures allows the compiler to determine the type and argument types of imported procedures.

The \crexx{} level b standard library namespace is `rxfnsb`, to access these procedures programs should import this namespace.

```rexx
import rxfnsb
```

Without it the BIFs cannot be found.
