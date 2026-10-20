#!/usr/bin/env python3
"""Build the NetRexx CSV source archive; never upload it or bundle a JVM/SDK."""
import argparse
import gzip
import io
from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parents[1]


def build(destination):
    members = {str(p.relative_to(ROOT / 'netrexx')): p
               for p in sorted((ROOT / 'netrexx/rexxcsv').glob('*.nrx'))}
    members.update({
        'NetRexxDemo.nrx': ROOT / 'examples/NetRexxDemo.nrx',
        'rexx-csv-README.md': ROOT / 'packaging/README-netrexx.md',
        'rexx-csv-SPEC.md': ROOT / 'SPEC.md',
        'rexx-csv-LICENSE.txt': ROOT / 'LICENSE',
    })
    with destination.open('xb') as output:
        with gzip.GzipFile(fileobj=output, filename='', mode='wb', mtime=0) as zipped:
            with tarfile.open(fileobj=zipped, mode='w', format=tarfile.USTAR_FORMAT) as archive:
                for name, source in sorted(members.items()):
                    data = source.read_bytes()
                    if name.endswith('.nrx'):
                        if name.startswith('rexxcsv/'):
                            data = data.replace(b'../../LICENSE', b'../rexx-csv-LICENSE.txt')
                            data = data.replace(b'../../SPEC.md', b'../rexx-csv-SPEC.md')
                        else:
                            data = data.replace(b'../LICENSE', b'rexx-csv-LICENSE.txt')
                    info = tarfile.TarInfo(name)
                    info.size = len(data)
                    info.mode = 0o644
                    info.mtime = 0
                    archive.addfile(info, io.BytesIO(data))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path, help='new .rxpkg file, never overwritten')
    args = parser.parse_args()
    build(args.output)
    print('Created', args.output.resolve())
