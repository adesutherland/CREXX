# Security Policy

CREXX is no longer treated as a proof of concept, but Release 1 is still in
beta. Security reports are welcome and will be triaged, with fixes prioritized
according to impact and release risk.

## Supported Versions

| Version | Security status |
| --- | --- |
| `1.0.0-beta.2` | Supported for beta security triage and fixes |
| `1.0.0-beta.1` | Superseded by beta 2; use beta 2 unless reproducing a beta 1-only issue |
| Test tags and older development snapshots | Not supported |

Until the first stable Release 1, security fixes may be delivered on `develop`
first and then included in the next beta or release tag. There is no long-term
support branch for beta builds.

## Security Model

CREXX is a native compiler, assembler, linker, virtual machine, runtime, and
plugin system. The beta does not provide a sandbox for untrusted programs.

Important beta security assumptions:

- Do not run untrusted CREXX source, RXAS assembly, RXBIN bytecode, or native
  plugins.
- Native plugins execute in the host process and have the privileges of that
  process.
- File, process, network, and host-integration features use the normal
  permissions of the current operating-system user.
- Compiler exits and ADDRESS environments are extension points. Treat them as
  trusted code unless a future release documents a stronger boundary.
- macOS beta 2 release packages use Developer ID signing and notarization when
  the release workflow has the required Apple secrets. The stapled `.pkg`
  installer is the preferred macOS user package when present.
- Windows beta 2 packages should use the signed ZIP after the maintainer signing
  flow has run. Unsigned Windows ZIPs may exist as intermediate CI assets before
  signing is completed.

The project welcomes hardening work, especially around malformed source,
malformed bytecode, plugin loading, native API boundaries, file handling, and
resource exhaustion.

## Reporting A Vulnerability

If the issue can be discussed publicly, open a normal GitHub issue.

If the report contains exploit details, a crash trigger that should not be made
public immediately, or other sensitive information:

1. Use GitHub private vulnerability reporting for this repository if it is
   available.
2. If private vulnerability reporting is not available, open a minimal public
   issue saying that you need a private security contact. Do not include the
   sensitive details in that issue.

Useful report contents:

- Affected CREXX version or commit.
- Operating system and CPU architecture.
- Whether the issue affects source compilation, RXAS assembly, RXBIN loading,
  native plugins, `crexxsaa`, a standard library, or release packaging.
- A minimal reproducer, if it can be shared safely.
- Expected impact, for example crash, memory corruption, sandbox escape
  assumption, arbitrary file access, command execution, or denial of service.
- Whether the issue is already public or known to be exploited.

Security fixes will normally be developed privately when the report is
sensitive, then disclosed through release notes or a GitHub advisory when a
fixed build is available.
