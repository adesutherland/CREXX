# Warning-free builds and quiet Level G output

CP-C-FIND-06 is implemented and qualified locally on `develop`, unpublished.
The [authoritative plan](../../ROADMAP.md#proposed-c-first-extraction-phase--2026-10-10)
records the scope. [quiet-qualification.json](quiet-qualification.json) retains
source and log hashes for both existing build profiles.

Observed C++20 deprecations came from `filesystem::u8path`. The shared package
helper now constructs a path from `u8string` under C++20 and keeps the existing
C++17 route. Both standard-specific UTF-8 tests retain byte-exact plain and
non-ASCII path behavior. RexxScript, RXPP, activation-argument and vector
fixtures use unambiguous local binding names; the unused vector import is
removed. Local behavior, callable ABI and metadata formats are retained.
All existing RexxDoc blocks are unchanged. No blanket warning suppression was
added.

CMake's Level G launcher now saves successful dependency-wave output to
`stage-g/work/builder.stdout.log` and `builder.stderr.log`. Normal successful
builds omit the extra WAVE/START/BUILT/PUBLISHED chatter. Standard build-tool
progress remains. Unexpected stderr is shown; failures print their nonzero
status, diagnostic tails and full log paths. `CREXX_LEVEL_G_BUILD_VERBOSE=ON`
prints successful decisions, and direct builder invocation keeps its diagnostics.

Debug and Release `all` builds both pass with **zero warnings and zero custom
success-chatter lines**. The real builder's complete stdout is retained, with
empty stderr. Explicit verbose cached replay passes and exposes its decisions.
A controlled missing-source invocation returns 15 and preserves its NOTREADY
panic diagnostic; failure output is not swallowed.

Affected preprocessor, RexxScript, activation-argument, vector, C++17/C++20
package-path and neighboring negative-runtime controls pass **37/37 in Debug
(11.98 seconds) and 37/37 in Release (5.76 seconds)**. The two open-stdin PULL
cases have separate [proof](stdin-repair.md). Wider unchanged evidence is reused;
no new performance or full-suite result is claimed.

SAN-011, SAN-012 and SAN-QA-018 remain repaired, open and release-blocking with
Codex at the next Release 1 QA gate. These source warning/diagnostic changes do
not close those gates or alter the agreed metadata reader boundary.
