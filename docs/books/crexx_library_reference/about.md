# About the Library Reference

This publication contains the documentation of the cRexx standard library. All documented functionality is delivered in every distribution of the cRexx system, while platform dependent differences or supersets are relegated to the appendices. The Rexx built-in functions, traditionally seen as part of the language, are also documented in the *Language Reference*, but in this *Library Reference* there is an emphasis on platform dependent functions and cRexx specific additions.

With Classic Rexx having its origins as a language intended for command processing and procedural programming, two object oriented successors, Object Rexx and \nr{} have introduced (divergent) object concepts, types and syntax. cRexx introduced native hardware types[^java] and an object oriented notation which stays closer to Classic Rexx (with standard Classic Rexx labels and `class` and `interface` keywords in the location of the `procedure` keyword. In cRexx, the `stem` type is implemented as a class, part of this library, and useable in procedural programs as any other type. In addition to this, a large number of other data structures are documented here, and are considered part of the runtime library instead of components of the core language. This has enabled the cRexx team to deliver a set of data structures recognisable to users of both predecessors.

\begin{shaded}
This level of the Library does not use inheritance and polymorphic calls (overloaded signatures). If and when this is the case, this message will be removed and method calls will be losing type info. Removal of typed method names will have a deprecation period. The current APIs cannot be regarded as stable in this regard. To avoid a large change, interfaces that accept `.string` type will leave out the type name and will be the stable name.
\end{shaded}

Also to be found here are native implementations of Graphic User interfaces and database drivers, both *universal* like ODBC or specific drivers for engines like *sqlite* - next to cRexx implementations of keystores or interface libraries. 

An explicit design goal was to keep the standard library structure relatively flat, i.e. without deep hierarchies which need to be included and searched, and which make navigation of the library source and comparison of like classes complicated. The categories of the library components are intended to limit the search scope fairly quickly.

The top level of the library is:  

- OS
- Data
- Time
- GUI
- Math

\begin{shaded}
The current Release status of this Standard Library is that it is preliminary reference material rather than a part of the Release 1 beta baseline language contract. The user has to verify components, modules and drivers before
depending on these API's. All documented API's have implementations - planned functionality in here is clearly marked.
\end{shaded}

[^java]: NetRexx types are still Java VM types;

## Note about implementation

The contents of the library are put together using different technologies, although great care has been taken, using the `namespace` and `import` mechanisms, to hide that fact from the user. A large part is implemented in the cRexx language itself, and when that was impossible because of instruction set architecture or operating system dependencies, using plugins in the c language. So a library is just a library, until, mostly in large cross-platform applications, packaging of statically linked object libraries versus shared object libraries becomes an issue. All components can be re-packaged with the `rxlink` utility, while all the modules and libraries can be completely rebuilt from a source distribution or a clone of the cRexx git repository.
