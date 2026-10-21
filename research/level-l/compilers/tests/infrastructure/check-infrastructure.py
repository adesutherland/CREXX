#!/usr/bin/env python3
"""Focused Group 4 consumer checks using maintained native C fixture oracles.

This reuses the warm B/L fixture service and controlled condition/TRACE checks.
It neither invokes native C nor runs the compiler-wide qualification matrix.
"""
import argparse
from contextlib import nullcontext
import hashlib
import json
from pathlib import Path
import re
import runpy
import tempfile


HERE = Path(__file__).resolve().parent
CONDITIONS = runpy.run_path(str(HERE.parent / 'conditions-observations/check-conditions.py'))
STEMS = ('levelc_say_instruction', 'levelc_numeric_lostdigits',
         'levelc_arg_recursive_private', 'levelc_trace_values')


def context_shape(path):
    """Immutable context loads add no intermediate cell or local COPY.

    Real caller argument copies remain in the distinct call window. This checks
    the producer defect independently of a particular register/frame snapshot.
    """
    text = path.read_text()
    header, end = CONDITIONS['function_extent'](text)
    body = text[header.start():end]
    bindings = re.findall(r'^copy (r\d+),a1$', body, re.M)
    assert len(bindings) == 1, ('one admitted context binding', bindings)
    bases = {int(value) for value in re.findall(r'^call [^\n]*,r(\d+)$', body, re.M)}
    assert len(bases) == 1, ('one bounded caller window', bases)
    base = bases.pop()
    copies = re.findall(r'^copy r(\d+),' + bindings[0] + '$', body, re.M)
    assert copies and all(int(destination) > base for destination in copies), (
        'only real call arguments copy the immutable context', base, copies)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--prepared', type=Path)
    parser.add_argument('--inspect', type=Path)
    parser.add_argument('--phase', choices=('representative', 'all'), default='all')
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    if args.prepared:
        args.prepared.mkdir(parents=True, exist_ok=True)
    manager = (nullcontext(str(args.prepared.resolve())) if args.prepared else
               tempfile.TemporaryDirectory(prefix='infrastructure-', dir=args.work))
    with manager as temporary:
        directory = Path(temporary)
        batch = CONDITIONS['Batch'](args, directory)
        sources = [CONDITIONS['SOURCES'] / (stem + '.rexx') for stem in STEMS]
        identity = {str(source): hashlib.sha256(source.read_bytes()).hexdigest()
                    for source in sources}
        record = directory / 'infrastructure-inputs.json'
        if record.exists():
            previous = json.loads(record.read_text())
            # Test assertions run anew; changing them does not invalidate an
            # unchanged prepared compiler or native fixture source.
            previous.pop(str(Path(__file__).resolve()), None)
            assert previous == identity, 'stale prepared infrastructure sources'
        else:
            record.write_text(json.dumps(identity, sort_keys=True), encoding='utf-8')
        # Review one representative protected/call program early; reuse its
        # exact unchanged preparation during the remaining selected checks.
        receipt = directory / 'representative.passed'
        if not receipt.exists() or receipt.read_text() != batch.representative_identity():
            batch.representative()
        context_shape(batch.application / (CONDITIONS['DELAYED']['policy'] + '.rxas'))
        if args.phase == 'all':
            batch.prepare(STEMS)
            for stem in STEMS[:-1]:
                source = batch.application / (stem + '.rxas')
                batch.inspect(source)
                batch.checked_run(stem, batch.link(source), CONDITIONS['ordinary_output'](stem))
            batch.trace()
            batch.contracts()
        print(f'INFRASTRUCTURE_CONSUMERS|phase={args.phase}|commands={batch.commands}|PASS')


if __name__ == '__main__':
    main()
