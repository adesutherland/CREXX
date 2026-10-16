# RXAS branch-label convergence

The PD-033 native spin-program build exposed nontermination during label
backpatch optimization. A minimal Level B `do forever; nop; end` reproduces
it on the host. Sampling places execution in `optimise_labels`: two distinct
labels share a branch address, and copying that unchanged address still sets
`changed`. The intended outcome is a terminating assembler that preserves the
existing infinite-loop semantics, followed by a successful native build and
cancellation of the actual running program. No language or bytecode-format
change is proposed. Unrelated Level L work remains untouched.

## Acceptance

1. **AC-01 (host pass):** Alias/self and multi-label branch cycles assemble with
   optimization enabled and disabled under a generous hang watchdog. The
   disassembled result retains a valid loop; finite branch chains still reach
   their expected return. Focused normal/sanitizer controls verify this.
2. **AC-02 (pass):** Existing affected assembler correctness checks and host
   compiler/assembler builds pass without new diagnostics. The empty-loop
   cREXX fixture remains unchanged.
3. **AC-03 (native pass):** Reviewed rebuilt native consumers and exact package inputs
   pass the affected native z/OS qualification before PDOS starts. The spin
   build completes, and cancellation targets its actual RUN step. Record
   boundaries and reuse unaffected evidence.

## Steps

1. **STEP-01:** Guard real target-index changes and valid instruction bounds
   in label optimization (AC-01). Add decisive regression inputs.
2. **STEP-02:** Measure the small check independently in normal Debug and the
   maintained sanitizer route before registering its scheduling/timeout; run
   affected correctness checks and review the repair (AC-01/02).
3. **STEP-03:** Rebuild affected native consumers in the existing normal batch
   paths, record exact source/artifact freeze and coordinate operator-only
   native requalification, then PDOS acceptance (AC-03).

The native baseline SPINBLD JOB00639 compiled successfully but its ASSEM step
was cancelled with S222. This is a failed build, not the planned RUN
cancellation proof. Other native case outcomes remain separate evidence.

The reviewed repair passes all 200 affected normal RXAS tests and three focused
Apple ASan tests. The normal regression takes about 0.04 seconds; its Apple
ASan run takes about 1.78 seconds. Native RXC/RXAS rebuilds pass the existing
entry/ISA checks; only `assembler/rxasassm.c` differs in their actual source
closure. The selected-consumer package rebuild preserves all other tools and
the frozen shared libraries. Native replacement/affected-case qualification
passed before PDOS starts. Existing flat-intermediate ELF RWX warnings
and unrelated Apple sprintf deprecations remain visible and are not suppressed.

Native z/OS 1.5 JOB00643 completed SPINBLD COMP/ASSEM 0/0. Replacement CG and
PARSE completed 0/0/0 with their exact application markers. JOB00646 reached
the actual SPIN RUN step, progressed from CPU19.249s to28.993s, then ended S222
on cancellation of that exact job. Post-cancel CPROBE JOB00647 passed RC0 and
the ordered binary readback. The exported SPIN is 1,124 bytes and disassembles
to SAY followed by a self-branch. The coordinator independently accepted these
records and the restored output policy. Paired PDOS R5 integration also passes
independent acceptance: 33 line and 37 model-2 checks, each with 68 commands and
17 jobs, including successful SPIN build and cancellation of the actual running
program. All five retained output hashes equal native z/OS. The separate native
console-label codepage repair also passes its focused checks, and final handback
closes the owning z-pdos Stage 4 gate. Those checks reuse these unchanged
assembler/runtime inputs. Publication and release remain separate gates.
