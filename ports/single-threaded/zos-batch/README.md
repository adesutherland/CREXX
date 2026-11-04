# cREXX TSO31 batch proof of concept

This is the October 2026 qualified batch payload, packaged for installation
on z/OS. It is newer than the beta.3 mainframe package, but is **not a new
general cREXX release**. It contains RXC, RXAS, RXLINK and RXVM load modules,
matching RXBIN007 libraries, four batch procedures, source and example jobs.
The native tool heaps are 192 MiB; the procedures request REGION=256M.

The ZIP contains one binary `CREXXB.XMI`, this guide, `INSTALL.jcl`, notices
and SHA-256 checksums. The outer XMI restores an FB80 distribution PDS; its
LOAD, RXBIN, PROCLIB, SOURCE and JCL members are nested XMIT files. INSTALL
and the procedure/job templates are also ordinary members of that PDS.

1. Check `SHA256SUMS`. Transfer CREXXB.XMI **in binary mode** to a new
   sequential dataset with RECFM=FB, LRECL=80, BLKSIZE=3120 and sufficient
   primary/secondary space (1000 tracks on 3390 is ample). Do not translate,
   add RDWs or transfer it as text. With IND$FILE, preallocate the dataset
   and choose binary transfer without CRLF conversion.
2. At TSO READY, receive it into a **new** distribution PDS:

   ```text
   RECEIVE INDSNAME('YOUR.CREXX.XMI')
   DATASET('YOUR.RECEIVED.DIST')
   ```

   The second line answers RECEIVE's restore-parameters prompt. On non-SMS
   systems add your site's `UNIT(...) VOLUME(...)` to that answer. This is
   standard z/OS TRANSMIT/RECEIVE, not the historical MVS 3.8 RECV370 route.
3. Transfer INSTALL.jcl as **text** (FB80, normal site EBCDIC), or paste its
   cards into an ISPF JCL member. Edit its JOB card, both distribution DDs
   and all five CONFIG target names. Use unquoted, fully qualified names in
   CONFIG. LOAD, PROCLIB, SOURCE and JCL may be named independently. This
   qualified compiler's import resolver requires the library dataset to
   end in `.RXBIN`; its root is derived automatically. No user-ID prefix or
   laboratory dataset name is built into the installer.
4. Select UNIT=SYSDA for normal site allocation, or specify UNIT=3390 and
   VOLUME=your-volume for non-SMS storage. The installer carries that choice
   into temporary dataset allocations in the installed procedures. All five
   target datasets must be absent. Submit INSTALL.jcl. Expect CC 0000 and
   `cREXX BATCH INSTALL OK` in SYSTSPRT. No target is replaced or deleted.
   A failure reports partial targets; review those before using fresh names.
   Do not run concurrent installers against the same names.
5. In the installed JCL library, edit the site JOB card in IVP (accounting,
   class and output disposition). Submit IVP. COMP, ASSEM, LINK and RUN must
   each return CC 0000. RUN's SYSPRINT must contain:

   ```text
   cREXX BATCH IVP OK: new payload
   ```

   The IVP calls `changestr` from the matching LIBRARY. Temporary assembly,
   object and linked bytecode datasets use PASS/DELETE. There is no separate
   assembler, binder, C runtime installation or per-tool TSO CALL sequence.

The installed procedures are CRXC (compile/assemble), CRXCG
(compile/assemble/run), CRXCL (compile/assemble/link), and CRXLGO
(compile/assemble/link/run). C and CL examples require a separately allocated
VB260 output dataset and an edited OUT parameter. CG runs unlinked bytecode
with LIBRARY; IVP runs linked bytecode. BAD is an intentional negative job:
COMP must fail and ASSEM/LINK/RUN must be skipped. A second submission of
INSTALL with the same targets must return CC 0012 before changing anything.

To use your own program, create or edit a source member using ISPF's normal
text facilities (preserve case, no stored sequence columns). Copy IVP to a
new job, change COMP.SRC to that member, and set ARGS as needed. The installed
PROCs have the selected library defaults. JOBLIB selects the native tools;
JCLLIB selects the procedures. When choosing a different matching library
installation, override ROOT; for CRXCG, CRXCL and CRXLGO also override RXLIB.
Use ordinary unquoted dataset/member names on JCL DD cards, for example
`DSN=YOUR.SOURCE(HELLO)`. TSO commands use their own quoting conventions.

SYSIN is the program's input; SYSPRINT carries ordinary output; SYSTERM
carries diagnostics. All are native DD-backed batch streams. These tools
do not use TPUT for ordinary batch output. Binary bytecode is VB260; it is
not a native load module and does not belong in SYSLMOD. Text records use
IBM1047 at the guest boundary, UTF-8 internally. Binary XMI and RXBIN content
must never undergo text translation. Use your site's held output class and
SDSF/OUTPUT facilities to retain the complete job and per-step spool output.

This is the direct EXEC batch route. `ADDRESS TSO` additionally requires a
background TSO environment (IKJEFT01) and its normal SYSTSIN/SYSTSPRT DDs;
it is not supplied merely by EXEC PGM=RXVM. Foreground TSO, USS and historical
MVS variants are outside this package's installation verification.

Qualification of the unchanged October payload was on z/OS 1.5 and PDOS.
The recipient installation is tested separately using this distribution.
z/OS 1.13 remains a recipient check. See the delivery note for exact new
installation/IVP results; do not infer a newer compiler baseline from the
packaging date. The supplied component notices retain their original terms.

IBM references: [TRANSMIT operands](https://www.ibm.com/docs/en/zos/2.5.0?topic=command-transmit-operands)
and [SYSDSN](https://www.ibm.com/docs/en/zos/3.2.0?topic=tef-sysdsn).
