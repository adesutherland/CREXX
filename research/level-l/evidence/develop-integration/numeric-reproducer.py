import hashlib, json, subprocess
from pathlib import Path
root = Path(__file__).resolve().parents[4]
bin_dir = root / 'cmake-build-levell-product/bin'
out = Path(__file__).resolve().parent / 'reproducer'
out.mkdir(exist_ok=True)
source = root / 'rexxscript/tests_functional/float-comparison-reproducer.rexx'
records = []

def run(command):
    command = [str(x) for x in command]
    result = subprocess.run(command, capture_output=True, text=True, timeout=300)
    records.append(dict(command=command, exit=result.returncode, stdout=result.stdout, stderr=result.stderr))
    assert result.returncode == 0, records[-1]
    return result.stdout

for mode in ['opt', 'noopt']:
    flags = ['-n'] if mode == 'noopt' else []
    image = out / mode
    run([bin_dir/'rxc', '-x', '--no-exe-import', *flags, '-i', bin_dir, '-o', image, source])
    run([bin_dir/'rxas', *flags, image])
    linked = out / (mode + '-linked')
    run([bin_dir/'rxlink', '-s', '-o', linked, image.with_suffix('.rxbin')])
    for vm in ['rxtvm', 'rxbvm']:
        observed = run([bin_dir/vm, linked.with_suffix('.rxbin')])
        (out/(mode+'-'+vm+'.out')).write_text(observed)
        assert observed == '0\n', (mode, vm, observed)
(out/'receipt.json').write_text(json.dumps(dict(cells=4, expected='0', commands=records,
    source_sha256=hashlib.sha256(source.read_bytes()).hexdigest()), indent=2)+'\n')
print('RS-F01 reproducer: 4/4 cells return 0')
