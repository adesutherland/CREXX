#!/usr/bin/env python3
"""Expose a standalone Level L build to the same product-surface test drivers.

Only the native C tools and Level B prerequisite come from the host. Every
Level L module, the consolidated runtime and the generator use the selected
standalone build (including its optimization setting).
"""
import argparse
import hashlib
import json
import shlex
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('host_bin', type=Path)
p.add_argument('standalone_build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
host, build, out = a.host_bin.resolve(), a.standalone_build.resolve(), a.output.resolve()
binary = out / 'bin'
binary.mkdir(parents=True, exist_ok=True)
inputs = {}


def link(source, target):
    assert source.is_file() or source.is_dir(), source
    if target.is_symlink():
        target.unlink()
    assert not target.exists(), target
    target.symlink_to(source)
    if source.is_file():
        inputs[str(target.relative_to(out))] = dict(source=str(source), sha256=hashlib.sha256(source.read_bytes()).hexdigest())


for name in ['rxc', 'rxas', 'rxlink', 'rxtvm', 'rxbvm']:
    link(host / name, binary / name)
for source in (build / 'rexx').glob('*.rxbin'):
    link(source, binary / source.name)
for name in ['library.rxbin', 'rxfnsl.rxbin', 'rxlanglib.rxbin']:
    link(build / 'bin' / name, binary / name)
stages = build / 'langtools/bootstrap'
for name in ['llfront.rxbin', 'llsyntax.rxbin']:
    link(stages / 'L2' / name, binary / name)
tool = stages / 'L2/rxlang-linked.rxbin'
launcher = binary / 'rxlang'
launcher.write_text('#!/bin/sh\nexec ' + shlex.quote(str(host / 'rxtvm')) + ' '
                    + shlex.quote(str(tool)) + ' -a "$@"\n')
launcher.chmod(0o755)
inputs['generator'] = dict(source=str(tool), sha256=hashlib.sha256(tool.read_bytes()).hexdigest())
stage_link = out / 'langtools/bootstrap'
stage_link.parent.mkdir(parents=True, exist_ok=True)
link(stages, stage_link)
property_link = out / 'lib/rxfnsl/rexx/properties'
property_link.parent.mkdir(parents=True, exist_ok=True)
link(build / 'rexx/properties', property_link)
(out / 'inputs.json').write_text(json.dumps(inputs, indent=2) + '\n')
print(out)
