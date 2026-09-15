"""Assemble the pinned, minimal CUDA build SDK from NVIDIA redistributables."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import tarfile
import tempfile
import urllib.request
import zipfile


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--root', type=Path, required=True)
    p.add_argument('--platform', choices=['linux-x86_64', 'windows-x86_64'], required=True)
    a = p.parse_args()
    root = a.root.resolve()
    lock_path = Path(__file__).resolve().parents[1] / '.github/llama/cuda-12.9.1.json'
    lock = json.loads(lock_path.read_text())
    identity = hashlib.sha256(lock_path.read_bytes()).hexdigest() + ':' + a.platform
    marker = root / '.crexx-cuda-lock'
    if not marker.exists() or marker.read_text() != identity:
        root.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='crexx-cuda-') as temp:
            temp = Path(temp)
            for name, platforms in lock['components'].items():
                entry = platforms[a.platform]
                archive = temp / Path(entry['relative_path']).name
                url = 'https://developer.download.nvidia.com/compute/cuda/redist/' + entry['relative_path']
                print('Provisioning ' + name, flush=True)
                urllib.request.urlretrieve(url, archive)
                with archive.open('rb') as stream:
                    digest = hashlib.file_digest(stream, 'sha256').hexdigest()
                if archive.stat().st_size != int(entry['size']) or digest != entry['sha256']:
                    raise RuntimeError('CUDA archive identity mismatch: ' + name)
                unpack = temp / name
                if archive.suffix == '.zip':
                    with zipfile.ZipFile(archive) as z:
                        z.extractall(unpack)
                else:
                    with tarfile.open(archive) as t:
                        t.extractall(unpack, filter='data')
                children = list(unpack.iterdir())
                if len(children) != 1 or not children[0].is_dir():
                    raise RuntimeError('Unexpected NVIDIA archive layout: ' + name)
                component = children[0]
                for item in component.iterdir():
                    if item.is_dir():
                        shutil.copytree(item, root / item.name, dirs_exist_ok=True, symlinks=True)
                    else:
                        destination = root / 'licenses' / name / item.name
                        destination.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(item, destination)
                archive.unlink()
                shutil.rmtree(unpack)
        # FindCUDAToolkit expects the conventional Unix SDK layout.
        if a.platform.startswith('linux') and (root / 'lib').exists():
            (root / 'lib64').symlink_to('lib', target_is_directory=True)
        marker.write_text(identity)
    nvcc = root / 'bin' / ('nvcc.exe' if os.name == 'nt' else 'nvcc')
    if not nvcc.is_file():
        raise RuntimeError('Pinned SDK has no nvcc: ' + str(nvcc))
    notices = ['NVIDIA CUDA ' + lock['version'] + ' redistributable dependency notices',
               'Source: ' + lock['source'], '']
    for name in lock['components']:
        license_file = root / 'licenses' / name / 'LICENSE'
        if not license_file.is_file():
            raise RuntimeError('CUDA component license missing: ' + name)
        notices.extend([name, license_file.read_text(encoding='utf-8'), ''])
    notice_path = root / 'crexx-cuda-NOTICES.txt'
    notice_path.write_text('\n'.join(notices), encoding='utf-8')
    if 'GITHUB_ENV' in os.environ:
        with open(os.environ['GITHUB_ENV'], 'a') as f:
            f.write('CUDA_PATH=' + str(root) + '\nCUDAToolkit_ROOT=' + str(root) + '\n'
                    'CREXX_LLAMA_CUDA_NOTICE=' + str(notice_path) + '\n')
        with open(os.environ['GITHUB_PATH'], 'a') as f:
            f.write(str(root / 'bin') + '\n')
    print('CUDA SDK ready: ' + str(root))


if __name__ == '__main__':
    main()
