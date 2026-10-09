#!/usr/bin/env python3
"""Exercise compiled CALL traps with a controlled pending event in RXAS.

ADDRESS and host HALT producers have separate Level C owners. These tests add
only controlled queue operations near authored clauses; handler policy,
clause-end delivery, invocation, CONDITION and RESULT behavior is product code.
"""

import argparse
from pathlib import Path
import re
import subprocess
import os


def run(command, cwd, log_path, expect_failure=False):
    process = subprocess.Popen(
        command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        output, _ = process.communicate(timeout=120)
    except subprocess.TimeoutExpired:
        process.kill()
        process.communicate()
        raise RuntimeError(f"timed out: {command[0]}")
    log_path.write_bytes(output)
    if (process.returncode != 0) != expect_failure:
        raise RuntimeError(
            f"{command[0]} exited {process.returncode}; see {log_path}: "
            + output[-2000:].decode("utf-8", errors="replace"))
    return output.replace(b"\r\n", b"\n") if os.name == "nt" else output


def queue_snippet(base, line, description, condition=2):
    return (
        f'   load r{base},4\n'
        f'   settp a3,256\n'
        f'   swap r{base + 1},a3\n'
        f'   load r{base + 2},{condition}\n'
        f'   load r{base + 3},"{description}"\n'
        f'   load r{base + 4},{line}\n'
        f'   settp r{base + 3},768\n'
        f'   call r{base + 5},§rexxactivation.rexxactivationarguments.queuecallcondition(),r{base}\n'
        f'   swap a3,r{base + 1}\n')


def append_queue_import(assembly):
    if "§rexxactivation.rexxactivationarguments.queuecallcondition() .expose=" in assembly:
        raise RuntimeError("queue import unexpectedly exists before injection")
    return assembly + (
        "\n§rexxactivation.rexxactivationarguments.queuecallcondition() "
        ".expose=rexxactivation.rexxactivationarguments.queuecallcondition\n"
        '   .meta "rexxactivation.rexxactivationarguments.queuecallcondition"="b" '
        '".boolean" §rexxactivation.rexxactivationarguments.queuecallcondition() '
        '"condition=.int,description=.string,line=.int"\n'
        '   .meta "rexxactivation.rexxactivationarguments.queuecallcondition"='
        '".autoload" "rxfnsc"\n')


def inject_policy(assembly):
    body_start = assembly.index("__rxcp_levelc_body() .locals=")
    body_end = assembly.index(
        "__rxcp_levelc_call_trap_dispatch() .locals=", body_start)
    body = assembly[body_start:body_end]
    pattern = re.compile(
        r"   call r\d+,§rexxactivation\.rexxactivationarguments\.setcallpolicy\(\),r\d+\n"
        r"   swap a3,r\d+\n")
    matches = list(pattern.finditer(body))
    if len(matches) != 2:
        raise RuntimeError(f"expected two CALL ON policy sites, found {len(matches)}")
    maximum = max(int(value) for value in re.findall(r"\br(\d+)\b", body))
    for ordinal, match in reversed(list(enumerate(matches))):
        base = maximum + 1 + ordinal * 6
        line = 3 if ordinal == 0 else 5
        snippet = queue_snippet(base, line, "Élan 😀")
        body = body[:match.end()] + snippet + body[match.end():]
    body = re.sub(
        r"(__rxcp_levelc_body\(\) \.locals=)\d+",
        lambda match: match.group(1) + str(maximum + len(matches) * 6 + 1),
        body, count=1)
    return append_queue_import(assembly[:body_start] + body + assembly[body_end:])


def inject_boundaries(assembly, sites_description):
    body_start = assembly.index("__rxcp_levelc_body() .locals=")
    body_end = assembly.index(
        "__rxcp_levelc_call_trap_dispatch() .locals=", body_start)
    body = assembly[body_start:body_end]
    policies = list(re.finditer(
        r"   call r\d+,§rexxactivation\.rexxactivationarguments\.setcallpolicy\(\),r\d+\n",
        body))
    if len(policies) != len(sites_description):
        raise RuntimeError(
            f"expected {len(sites_description)} CALL ON policies, found {len(policies)}")
    maximum = max(int(value) for value in re.findall(r"\br(\d+)\b", body))
    sites = []
    for policy in policies:
        following = re.search(
            r"   call4 r\d+,__rxcp_levelc_call_trap_dispatch\(\),[^\n]*\n",
            body[policy.end():])
        if following is None:
            raise RuntimeError("CALL ON clause has no generated checkpoint")
        sites.append(policy.end() + following.end())
    for ordinal in reversed(range(len(sites))):
        base = maximum + 1 + ordinal * 6
        line, description = sites_description[ordinal]
        site = sites[ordinal]
        body = body[:site] + queue_snippet(base, line, description) + body[site:]
    body = re.sub(
        r"(__rxcp_levelc_body\(\) \.locals=)\d+",
        lambda match: match.group(1) + str(maximum + len(sites) * 6 + 1),
        body, count=1)
    return append_queue_import(assembly[:body_start] + body + assembly[body_end:])


def inject_after_signal_override(assembly, line, description):
    body_start = assembly.index("__rxcp_levelc_body() .locals=")
    body_end = assembly.index(
        "__rxcp_levelc_call_trap_dispatch() .locals=", body_start)
    body = assembly[body_start:body_end]
    policies = list(re.finditer(
        r"   call r\d+,§rexxactivation\.rexxactivationarguments\.setsignalpolicy\(\),r\d+\n",
        body))
    if not policies:
        raise RuntimeError("SIGNAL override has no policy write")
    checkpoint = re.search(
        r"   call4 r\d+,__rxcp_levelc_call_trap_dispatch\(\),[^\n]*\n",
        body[policies[0].end():])
    if checkpoint is None:
        raise RuntimeError("SIGNAL override has no generated checkpoint")
    site = policies[0].end() + checkpoint.end()
    maximum = max(int(value) for value in re.findall(r"\br(\d+)\b", body))
    body = body[:site] + queue_snippet(maximum + 1, line, description) + body[site:]
    body = re.sub(
        r"(__rxcp_levelc_body\(\) \.locals=)\d+",
        lambda match: match.group(1) + str(maximum + 7), body, count=1)
    return append_queue_import(assembly[:body_start] + body + assembly[body_end:])


def inject_lifecycle(assembly):
    body_start = assembly.index("__rxcp_levelc_body() .locals=")
    body_end = assembly.index(
        "__rxcp_levelc_call_trap_dispatch() .locals=", body_start)
    body = assembly[body_start:body_end]
    on = re.search(
        r"   call r\d+,§rexxactivation\.rexxactivationarguments\.setcallpolicy\(\),r\d+\n"
        r"   swap a3,r\d+\n", body)
    off = re.search(
        r"   call r\d+,§rexxactivation\.rexxactivationarguments\.setsignalpolicy\(\),r\d+\n"
        r"   swap a3,r\d+\n", body)
    first = re.search(r'   load r\d+,"first"\n', body)
    after_first = first and re.search(r"   say r\d+\n", body[first.end():])
    body_calls = list(re.finditer(
        r"   call4 r\d+,__rxcp_levelc_body\(\),[^\n]*\n", body))
    after_child = len(body_calls) == 2 and re.search(
        r"   call4 r\d+,__rxcp_levelc_call_trap_dispatch\(\),[^\n]*\n",
        body[body_calls[1].end():])
    if not on or not off or not first or not after_first or not after_child:
        raise RuntimeError("expected CALL ON, first SAY, CALL OFF and child CALL sites")
    child_checkpoint = body_calls[1].end() + after_child.start()
    sites = (on.end(), first.end() + after_first.end(), off.end(), child_checkpoint)
    if list(sites) != sorted(sites):
        raise RuntimeError("lifecycle sites are not in source order")
    maximum = max(int(value) for value in re.findall(r"\br(\d+)\b", body))
    details = ((3, "first"), (4, "again"), (6, "off"), (9, "parent"))
    for ordinal in reversed(range(len(sites))):
        base = maximum + 1 + ordinal * 6
        line, description = details[ordinal]
        site = sites[ordinal]
        body = body[:site] + queue_snippet(base, line, description) + body[site:]
    body = re.sub(
        r"(__rxcp_levelc_body\(\) \.locals=)\d+",
        lambda match: match.group(1) + str(maximum + len(sites) * 6 + 1),
        body, count=1)
    return append_queue_import(assembly[:body_start] + body + assembly[body_end:])


def inject_conditions(assembly):
    body_start = assembly.index("__rxcp_levelc_body() .locals=")
    body_end = assembly.index(
        "__rxcp_levelc_call_trap_dispatch() .locals=", body_start)
    body = assembly[body_start:body_end]
    markers = list(re.finditer(
        r'   \.srcstep \d+ \d+ 6 "levelc_call_condition_matrix\.rexx" '
        r'(\d+) 1 4 "nop"\n', body))
    sites = []
    for marker in markers:
        next_source = body.find("   .srcstep ", marker.end())
        segment = body[marker.end():next_source]
        dispatch = re.search(
            r"   call4 r\d+,__rxcp_levelc_call_trap_dispatch\(\),[^\n]*\n",
            segment)
        if dispatch is not None:
            sites.append((int(marker.group(1)), marker.end() + dispatch.start()))
    if [line for line, _ in sites] != [4, 7, 10, 13]:
        raise RuntimeError("expected one NOP checkpoint for each CALL condition")
    maximum = max(int(value) for value in re.findall(r"\br(\d+)\b", body))
    descriptions = ("error", "failure", "halt", "notready")
    for ordinal in reversed(range(4)):
        base = maximum + 1 + ordinal * 6
        line, site = sites[ordinal]
        body = body[:site] + queue_snippet(
            base, line, descriptions[ordinal], ordinal + 2) + body[site:]
    body = re.sub(
        r"(__rxcp_levelc_body\(\) \.locals=)\d+",
        lambda match: match.group(1) + str(maximum + 25),
        body, count=1)
    return append_queue_import(assembly[:body_start] + body + assembly[body_end:])


def inject_buffered_halt(assembly):
    body_start = assembly.index("__rxcp_levelc_body() .locals=")
    body_end = assembly.index(
        "__rxcp_levelc_call_trap_dispatch() .locals=", body_start)
    body = assembly[body_start:body_end]
    policy = re.search(
        r"   call r\d+,§rexxactivation\.rexxactivationarguments\.setcallpolicy\(\),r\d+\n",
        body)
    if policy is None:
        raise RuntimeError("HALT fixture has no CALL ON policy")
    checkpoint = re.search(
        r"   call4 r\d+,__rxcp_levelc_call_trap_dispatch\(\),[^\n]*\n",
        body[policy.end():])
    if checkpoint is None:
        raise RuntimeError("HALT policy has no generated checkpoint")
    first = policy.end() + checkpoint.end()
    marker = re.search(r'   load r\d+,"inside"\n', body)
    if marker is None:
        raise RuntimeError("HALT handler lacks the first-event marker")
    say = re.search(r"   say r\d+\n", body[marker.end():])
    if say is None:
        raise RuntimeError("HALT handler marker has no SAY")
    second = marker.end() + say.end()
    if first >= second:
        raise RuntimeError("HALT events are not in source order")
    maximum = max(int(value) for value in re.findall(r"\br(\d+)\b", body))
    body = body[:second] + queue_snippet(maximum + 7, 8, "buffered", 4) + body[second:]
    body = body[:first] + queue_snippet(maximum + 1, 3, "initial", 4) + body[first:]
    body = re.sub(
        r"(__rxcp_levelc_body\(\) \.locals=)\d+",
        lambda match: match.group(1) + str(maximum + 13), body, count=1)
    return append_queue_import(assembly[:body_start] + body + assembly[body_end:])


def main():
    parser = argparse.ArgumentParser()
    for name in ("rxc", "rxas", "rxvm", "bindir", "source", "workdir", "mode"):
        parser.add_argument(f"--{name}", required=True)
    parser.add_argument("--scenario", choices=("policy", "boundaries", "transfers", "lifecycle", "missing", "conditions", "buffered_halt"),
                        default="policy")
    args = parser.parse_args()
    args.rxc = str(Path(args.rxc).resolve())
    args.rxas = str(Path(args.rxas).resolve())
    args.rxvm = str(Path(args.rxvm).resolve())
    args.bindir = str(Path(args.bindir).resolve())
    args.source = str(Path(args.source).resolve())
    workdir = Path(args.workdir).resolve()
    workdir.mkdir(parents=True, exist_ok=True)
    stem = f"levelc_call_{args.scenario}_{args.mode}"
    assembly_path = workdir / f"{stem}.rxas"
    injected_path = workdir / f"{stem}_injected.rxas"
    binary_path = workdir / f"{stem}.rxbin"
    compile_command = [args.rxc, "-i", args.bindir]
    if args.mode == "noopt":
        compile_command.append("-n")
    elif args.mode != "opt":
        raise RuntimeError(f"unknown mode {args.mode}")
    compile_command += ["-o", str(assembly_path), args.source]
    run(compile_command, workdir, workdir / f"{stem}_compile.log")
    if args.scenario == "policy":
        injected = inject_policy(assembly_path.read_text(encoding="utf-8"))
    elif args.scenario == "conditions":
        injected = inject_conditions(assembly_path.read_text(encoding="utf-8"))
    elif args.scenario == "lifecycle":
        injected = inject_lifecycle(assembly_path.read_text(encoding="utf-8"))
    elif args.scenario == "buffered_halt":
        injected = inject_buffered_halt(assembly_path.read_text(encoding="utf-8"))
    elif args.scenario == "missing":
        injected = inject_boundaries(assembly_path.read_text(encoding="utf-8"), ((3, "missing"),))
    else:
        sites = ((4, "if"), (9, "when"), (15, "do")) if args.scenario == "boundaries" else (
            (4, "leave"), (11, "exit"), (17, "return"))
        injected = inject_boundaries(assembly_path.read_text(encoding="utf-8"), sites)
    injected_path.write_text(injected, encoding="utf-8")
    run([args.rxas, "-o", str(binary_path), str(injected_path)],
        workdir, workdir / f"{stem}_assemble.log")
    output = run([args.rxvm, str(binary_path),
                  str(Path(args.bindir) / "library"),
                  str(Path(args.bindir) / "classlib"),
                  str(Path(args.bindir) / "rxfnsc")],
                 workdir, workdir / f"{stem}_run.log",
                 expect_failure=args.scenario == "missing")
    if args.scenario == "missing":
        decoded = output.decode("utf-8", errors="replace")
        if "RXC-LC-16.1: Label not found: ABSENT" not in decoded:
            raise RuntimeError(f"missing delayed handler did not report 16.1: {decoded!r}")
        if "source: levelc_call_missing_delayed_handler.rexx:3:1: nop" not in decoded:
            raise RuntimeError(f"missing delayed handler source was not line 3: {decoded!r}")
        return
    expected_by_scenario = {
        "policy": (
            "caught=ERROR|Élan 😀|CALL|DELAY|3|0\n"
            "local-after=old|old|\n"
            "bif-after=old|old|\n"),
        "boundaries": (
            "trap=4|ERROR|if\n"
            "if-body\n"
            "trap=9|ERROR|when\n"
            "when-body\n"
            "trap=15|ERROR|do\n"
            "do-body\n"),
        "transfers": (
            "trap=4|ERROR|leave\n"
            "after-leave\n"
            "trap=17|ERROR|return\n"
            "after-return\n"
            "trap=11|ERROR|exit\n"),
        "lifecycle": (
            "caught=ERROR|first|DELAY|CALL|0|old|3\n"
            "first\n"
            "caught=ERROR|again|DELAY|CALL|0|old|4\n"
            "second\n"
            "off\n"
            "caught=ERROR|parent|DELAY|CALL|0|old|9\n"
            "after-child\n"
            "parent=|old\n"),
        "conditions": (
            "caught=ERROR|error||CALL|DELAY|4|0\n"
            "caught=FAILURE|failure||CALL|DELAY|7|0\n"
            "caught=HALT|halt||CALL|DELAY|10|0\n"
            "caught=NOTREADY|notready||CALL|DELAY|13|0\n"
            "after=prior|prior|\n"),
        "buffered_halt": (
            "caught=initial|3\n"
            "inside\n"
            "caught=buffered|8\n"
            "after\n"),
    }
    expected = expected_by_scenario[args.scenario].encode("utf-8")
    if output != expected:
        raise RuntimeError(f"unexpected output: {output!r}; expected {expected!r}")


if __name__ == "__main__":
    main()
