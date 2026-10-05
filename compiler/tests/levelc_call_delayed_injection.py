#!/usr/bin/env python3
"""Exercise compiled CALL traps with a controlled pending event in RXAS.

ADDRESS and host HALT producers have separate Level C owners. This test adds
only the queue operation at two authored CALL ON clauses; all handler policy,
clause-end delivery, invocation, CONDITION and RESULT behavior is product code.
"""

import argparse
from pathlib import Path
import re
import subprocess


def run(command, cwd, log_path):
    process = subprocess.Popen(
        command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        output, _ = process.communicate(timeout=120)
    except subprocess.TimeoutExpired:
        process.kill()
        process.communicate()
        raise RuntimeError(f"timed out: {command[0]}")
    log_path.write_bytes(output)
    if process.returncode:
        raise RuntimeError(
            f"{command[0]} exited {process.returncode}; see {log_path}: "
            + output[-2000:].decode("utf-8", errors="replace"))
    return output


def queue_snippet(base, line, description):
    return (
        f'   load r{base},4\n'
        f'   settp a3,256\n'
        f'   swap r{base + 1},a3\n'
        f'   load r{base + 2},2\n'
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


def main():
    parser = argparse.ArgumentParser()
    for name in ("rxc", "rxas", "rxvm", "bindir", "source", "workdir", "mode"):
        parser.add_argument(f"--{name}", required=True)
    parser.add_argument("--scenario", choices=("policy", "boundaries", "transfers"),
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
        injected = inject_policy(assembly_path.read_text())
    else:
        sites = ((3, "if"), (7, "when"), (12, "do")) if args.scenario == "boundaries" else (
            (4, "leave"), (11, "exit"), (17, "return"))
        injected = inject_boundaries(assembly_path.read_text(), sites)
    injected_path.write_text(injected, encoding="utf-8")
    run([args.rxas, "-o", str(binary_path), str(injected_path)],
        workdir, workdir / f"{stem}_assemble.log")
    output = run([args.rxvm, str(binary_path),
                  str(Path(args.bindir) / "library"),
                  str(Path(args.bindir) / "classlib"),
                  str(Path(args.bindir) / "rxfnsc")],
                 workdir, workdir / f"{stem}_run.log")
    expected = ((
        "caught=ERROR|Élan 😀|CALL|DELAY|3|0\n"
        "local-after=old|old|\n"
        "bif-after=old|old|\n") if args.scenario == "policy" else (
        "trap=3|ERROR|if\n"
        "if-body\n"
        "trap=7|ERROR|when\n"
        "when-body\n"
        "trap=12|ERROR|do\n"
        "do-body\n") if args.scenario == "boundaries" else (
        "trap=4|ERROR|leave\n"
        "after-leave\n"
        "trap=17|ERROR|return\n"
        "after-return\n"
        "trap=11|ERROR|exit\n")).encode("utf-8")
    if output != expected:
        raise RuntimeError(f"unexpected output: {output!r}; expected {expected!r}")


if __name__ == "__main__":
    main()
