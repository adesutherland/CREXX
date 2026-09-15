# Native typed surface review — 14 September 2026

Adrian asked whether the native interface can expose the proposed llama surface.
RXPA already declares class/interface/factory/method metadata through ADDCLASS,
ADDINTERFACE, ADDIMPLEMENTS, ADDFACTORY and ADDMETHOD. Both dynamic and static
declaration consumers support it. This is not limited to scalar procedures.
These macros declare contracts; they do not bind member bodies or construct
runtime objects by themselves. The existing `rxpa_classdecl` tests are compile
controls whose native placeholder procedures return strings, not proof of a
complete native class implementation.

The documented gap is a supported pure-C operation for constructing/stamping
a typed object. RXPA has native payload ownership and calls into existing Rexx
objects, but its current host-service table has no object-construction service.
The supported route remains a small Rexx factory/class shim delegating work to C.
See [native class declarations and construction](../../ai-context/CREXX_LIBS.md#5-declaring-native-classes-and-interfaces).

There is a useful nuance: `rxstats.regression` declares `.linearfit` and fills
RETURN using SETNATIVEPAYLOAD. Its concrete slope/intercept methods work. The
retained diagnostic checks whether that establishes full runtime class identity.
Both ordinary optimized and `rxc -n` builds report:

```text
native 2 1 .object 1
factory 2 1 .rxstats..linearfit 1
native via object .object 0
```

Columns are values, runtime type and initialized state; the last line tests
the native result as `.linearfit` after widening to `.object`. Correct numeric
access and initialization do not establish the concrete type needed for runtime
type tests/interface dispatch. This exposes an existing native-result identity
limitation in the stats example; it is not evidence that the full llama object
surface can be implemented by filling return payloads alone. No stats/RXPA fix
or new compatibility promise is made here.

`shape.crexx`, `opt.log`, `noopt.log` and `identity.json` retain the diagnostic.
It prints observations rather than serving as a repaired regression gate. Build
with the current Debug rxc (`-n` for the second run), rxas and rxlink, explicitly
linking `library` and `rxfnsg`; run with rxbvm. Generated images remain in the
scratch directory recorded in the identity file. This is a small functional
probe, not sanitizer, performance or full-suite work.

For S4-D03 the options are a hybrid public surface with a minimal Rexx
construction/method shim, or a separately selected generic RXPA construction
extension plus qualification of native member/factory bindings. Class-shaped
native metadata is already available. The public names do not require the
entire facade to be implemented in Rexx. A fully native implementation has not
been established by this review; no architecture change is authorized by the
question alone.
