#!/usr/bin/env python3
"""Verify and repack the recorded Onegram package for ARMv7. Local files only."""
import argparse
import hashlib
import io
from pathlib import Path, PurePosixPath
import struct
import tarfile

EXPECTED_SHA256 = '5b4256ad35f9a1354fe11a826946910c7cbb82c98d67c26618320c534b8db65f'
APP = PurePosixPath('Applications/Onegram.app')


def ar_member(blob, wanted):
    if not blob.startswith(b'!<arch>\n'):
        raise ValueError('Not a Debian ar archive')
    offset = 8
    while offset < len(blob):
        header = blob[offset:offset + 60]
        if len(header) != 60 or header[58:60] != b'`\n':
            raise ValueError('Invalid ar header')
        size = int(header[48:58])
        name = header[:16].decode('ascii').strip().rstrip('/')
        start = offset + 60
        if start + size > len(blob):
            raise ValueError('Truncated ar member')
        if name == wanted:
            return blob[start:start + size]
        offset = start + size + size % 2
    raise ValueError('Missing ar member: ' + wanted)


def armv7_slice(data):
    if data[:4] != b'\xca\xfe\xba\xbe':
        return data
    if len(data) < 8:
        raise ValueError('Truncated Mach-O header')
    count = struct.unpack_from('>I', data, 4)[0]
    if count > 32 or len(data) < 8 + count * 20:
        raise ValueError('Invalid Mach-O architecture table')
    for i in range(count):
        cpu, subtype, offset, size, _ = struct.unpack_from('>IIIII', data, 8 + i * 20)
        if cpu == 12 and subtype & 0xffffff == 9:
            if offset + size > len(data):
                raise ValueError('Truncated ARMv7 slice')
            return data[offset:offset + size]
    raise ValueError('Fat binary does not contain ARMv7')


def prepare(source, destination):
    blob = source.read_bytes()
    actual = hashlib.sha256(blob).hexdigest()
    if actual != EXPECTED_SHA256:
        raise ValueError('Input checksum mismatch; refusing to repack a different artifact')
    if destination.exists():
        raise ValueError('Output exists; choose a new path')
    payload = ar_member(blob, 'data.tar.gz')
    files = 0
    total = 0
    # Validate all paths and build in memory before creating the output file.
    buffer = io.BytesIO()
    with tarfile.open(fileobj=io.BytesIO(payload), mode='r:gz') as original:
        with tarfile.open(fileobj=buffer, mode='w:gz') as result:
            for member in original.getmembers():
                name = PurePosixPath(member.name)
                if name.is_absolute() or '..' in name.parts:
                    raise ValueError('Unsafe package path')
                if not member.isfile():
                    if member.isdir():
                        continue
                    raise ValueError('Unexpected non-regular package member')
                if not name.is_relative_to(APP):
                    raise ValueError('Unexpected file outside Onegram app')
                if name == APP / 'OnegramEmoji.pack':
                    continue
                data = armv7_slice(original.extractfile(member).read())
                member.size = len(data)
                member.name = str(name)
                result.addfile(member, io.BytesIO(data))
                files += 1
                total += len(data)
    with destination.open('xb') as output:
        output.write(buffer.getvalue())
    print(f'Input SHA-256 verified. Packed {files} files ({total:,} bytes uncompressed).')
    print(f'Output: {destination} ({destination.stat().st_size:,} bytes)')
    print('No phone was connected or modified.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('package', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    try:
        prepare(args.package, args.output)
    except (OSError, ValueError, tarfile.TarError) as error:
        parser.exit(1, f'Error: {error}\n')
