"""Real consumers and refusal boundaries for shared contract preparation."""
import argparse
import os
import json
from pathlib import Path
import tempfile
from contract_fixture import ContractFixture, ProviderSource

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--work', type=Path, required=True)
a = p.parse_args()
a.work.mkdir(parents=True, exist_ok=True)

string_source = "options levelc\nsay 'STRING'\nexit\n"
bytes_source = (b'/* header\x00comment */\noptions levelb\nimport fixture_provider\n'
                + b'say get_text()\nreturn 0\n')
provider = ProviderSource('fixture_provider.crexx',
                          b'options levelb\nnamespace fixture_provider expose get_text\n'
                          + b'/* provider\x00comment */\nget_text: procedure = .string\n'
                          + b"return 'i\x00m'\n", ('-x',))


def byte_case(fixture, auxiliary=provider, linked=False):
    expected = b'i\x00m\r\n' if os.name == 'nt' else b'i\x00m\n'
    fixture.case('bytes', bytes_source, expected=expected, classic=False,
                 filename='bytes.crexx', providers=(auxiliary,), linked=linked)


def inventory(fixture, linked=False):
    fixture.case('string', string_source, expected='STRING\n', linked=linked)
    byte_case(fixture, linked=linked)


def must_refuse(action, diagnostic):
    try:
        action()
    except AssertionError as error:
        if diagnostic not in str(error):
            raise AssertionError(f'Wrong refusal: {error}') from error
    else:
        raise AssertionError('Incomplete or changed preparation unexpectedly admitted')


with tempfile.TemporaryDirectory(prefix='contract-fixture-', dir=a.work) as temporary:
    root = Path(temporary)

    def consume():
        with ContractFixture(a.bindir, root, [Path(__file__)]) as fixture:
            inventory(fixture)

    with ContractFixture(a.bindir, root, [Path(__file__)], prepare=True) as fixture:
        inventory(fixture)
    consume()
    with ContractFixture(a.bindir, root, [Path(__file__)]) as fixture:
        inventory(fixture, linked=True)
    with ContractFixture(a.bindir, root / 'fresh', [Path(__file__)], fresh=True) as fixture:
        inventory(fixture, linked=True)

    # A missing declared image must not silently fall back to compilation.
    image = root / 'prepared/noopt/bytes/program.rxbin'
    image_bytes = image.read_bytes()
    image.unlink()
    must_refuse(consume, 'Missing/stale')
    image.write_bytes(image_bytes)

    # An interrupted publication cannot be treated as a usable preparation.
    manifest = root / 'prepared/manifest.json'
    manifest_bytes = manifest.read_bytes()
    manifest.unlink()
    must_refuse(consume, 'Missing/stale')
    manifest.write_bytes(manifest_bytes)
    partial = json.loads(manifest_bytes)
    # Match the publisher's host-native relative-path keys.
    del partial['artifacts'][str(image.relative_to(root / 'prepared'))]
    manifest.write_text(json.dumps(partial))
    must_refuse(consume, 'Missing/stale')
    manifest.write_bytes(manifest_bytes)

    # Auxiliary source bytes are live preparation inputs, not runtime guesses.
    source = root / 'sources/bytes/fixture_provider.crexx'
    source_bytes = source.read_bytes()
    source.write_bytes(source_bytes + b'/* changed provider */\n')
    must_refuse(consume, 'Missing/stale')
    source.write_bytes(source_bytes)

    def changed_provider(auxiliary):
        with ContractFixture(a.bindir, root, [Path(__file__)]) as fixture:
            byte_case(fixture, auxiliary)

    must_refuse(lambda: changed_provider(ProviderSource(provider.filename, provider.source, ())),
                'source/mode mismatch')
    must_refuse(lambda: changed_provider(ProviderSource(provider.filename, provider.source + b'\n', provider.flags)),
                'source/mode mismatch')
    consume()
    # A provider removed by a generator update must not survive in the next
    # cold preparation's source-import directory as an undeclared input.
    obsolete = root / 'sources/bytes/obsolete_provider.crexx'
    obsolete.write_bytes(provider.source)
    must_refuse(consume, 'Missing/stale')
    with ContractFixture(a.bindir, root, [Path(__file__)], prepare=True) as fixture:
        inventory(fixture)
    if obsolete.exists():
        raise AssertionError('Cold preparation retained an undeclared source provider')
    consume()
print('CONTRACT_FIXTURE|PASS')
