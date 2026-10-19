# Using rxbuild with this example

Build the command from the repository root:

~~~powershell
cmake --build cmake-build-release --target rxbuild --parallel 1
~~~

Run the basic variable example:

~~~powershell
.\cmake-build-release\bin\rxbuild.exe tools\rxbuild\example.build
~~~

Run the minimal literal-only example:

~~~powershell
.\cmake-build-release\bin\rxbuild.exe tools\rxbuild\simple.build
~~~

This plan has no variables and no DO OVER loop. It compiles one grammar,
compiles one reader, compiles build_demo.crexx with COMPILE PROGRAM, and runs
the resulting build_demo.rxbin program.

Run the recursive file-selection example:

~~~powershell
.\cmake-build-release\bin\rxbuild.exe tools\rxbuild\selection.build
~~~

The selection plan uses:

~~~text
FILES IN "${GRAMMAR_DIR}" RECURSIVE EXTENSION ".levell" BASENAMES;
~~~

It processes build, nested/deep, and tiny in sorted order. The nested grammar
is written to cmake-build-release/build/selected/nested/deep.rxbin.

COMPILE APPLICATION is the source-language phase. It invokes a declared
language compiler to translate the source file to generated cREXX, then
compiles that generated source to the requested application .rxbin. See the
MiniBASIC application sample for a complete example. A following CREATE EXE
phase can package that application as a native executable:

~~~text
CREATE EXE
  OUTPUT_DIR = "cmake-build-release/minibasic/primes"
  APPLICATION = "prime"
  OUTPUT = "prime";
~~~

The `.rxbin` remains the portable application image. Windows produces the
`.exe`; Linux and macOS produce an extensionless native executable using the
compiler configured for the current CREXX build.

`COMPILE RXPP` preprocesses an RXPP source into `OUTPUT.crexx` beneath
`OUTPUT_DIR`. If the source contains `##RXBUILD REQUIRED`, this is the
supported phase because it invokes RXPP with the internal `--rxbuild` mode.

For the complete user guide, see
[docs/rxbuild.md](../../docs/rxbuild.md).
