#!/usr/bin/env python3
"""Retain the current compiler failure; passing no-opt is an independent control."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
r=Path(__file__).resolve().parent
bins=Path(os.environ.get('CREXX_BIN_DIR',r/'build/frozen-tools-20261010'))
case=r/'evidence/c14-first-opt-failure'
results={}
for mode in ['opt','noopt']:
    build=r/f'build/c14-compiler-repro-{mode}'
    # Do not attribute stale VM output to a failed build or shell invocation.
    for name in ['rxtvm.out','rxbvm.out','c14-linked.rxbin']:
        (build/name).unlink(missing_ok=True)
    env=dict(os.environ,CREXX_BIN_DIR=str(bins),C14_SOURCE=str(case/'llc14.crexx'),C14_CASE_SOURCE=str(case/'driver.crexx'),C14_BUILD_DIR=str(build),C14_FIXTURES=str(case/'fixture.txt'),C14_EXPECTED=str(case/'expected.txt'))
    run=subprocess.run([str(r/'run-c14.sh'),mode],env=env,capture_output=True,text=True)
    (case/f'{mode}-command.out').write_text(run.stdout+run.stderr)
    results[f'{mode}-command']=run.returncode
    expected_status=8 if mode=='opt' else 0
    if run.returncode != expected_status or not (build/'rxtvm.out').exists():
        sys.exit(f'{mode}: unexpected build/harness/VM result {run.returncode}; inspect {mode}-command.out before attributing compiler behavior.')
    if mode=='opt':
        other=subprocess.run([str(bins/'rxbvm'),str(build/'c14-linked.rxbin'),'-a',str(case/'fixture.txt'),str(case/'expected.txt')],capture_output=True,text=True)
        (build/'rxbvm.out').write_text(other.stdout+other.stderr)
        results['opt-rxbvm']=other.returncode
    for vm in ['rxtvm','rxbvm']:
        (case/f'{mode}-{vm}.out').write_bytes((build/f'{vm}.out').read_bytes())
    print(mode,run.returncode)
sha=lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
record={'status':'open compiler-dependent finding; core repair not made','expected':{'opt-command':8,'opt-rxbvm':8,'noopt-command':0},'actual':results,'tools':{n:sha(bins/n) for n in ['rxc','rxas','rxlink','rxtvm','rxbvm','library.rxbin']},'sources':{str(p.relative_to(r)):sha(p) for p in [case/'llc14.crexx',case/'fixture.txt',case/'expected.txt',r/'src/llcore.crexx',r/'src/llhistory.crexx',r/'src/llc14cost.crexx',case/'driver.crexx',r/'run-c14.sh',Path(__file__)]}}
(case/'provenance.json').write_text(json.dumps(record,indent=2)+'\n')
if results != record['expected']: sys.exit('Compiler behavior changed; inspect before revising this finding.')
print('Compiler finding reproduced in both VMs; no-opt controls pass.')
