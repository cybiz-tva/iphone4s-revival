#!/usr/bin/env python3
"""Prepare a Onegram Info.plist locally; never connects to a phone."""
import argparse
import plistlib
from pathlib import Path
from xml.parsers.expat import ExpatError

BUNDLE_ID = "dev.IllyaGIF.Onegram"


def prepare(source: Path, destination: Path) -> None:
    if source.resolve() == destination.resolve():
        raise ValueError("Use a different output path; preserve the original Info.plist")
    raw = source.read_bytes()
    info = plistlib.loads(raw)
    if not isinstance(info, dict) or info.get("CFBundleIdentifier") != BUNDLE_ID:
        raise ValueError("Input is not Onegram's Info.plist")
    info["SBAppUsesLocalNotifications"] = True
    fmt = plistlib.FMT_BINARY if raw.startswith(b"bplist00") else plistlib.FMT_XML
    # Refuse to overwrite a backup or a previously prepared file.
    prepared = plistlib.dumps(info, fmt=fmt, sort_keys=False)
    with destination.open("xb") as output:
        output.write(prepared)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    try:
        prepare(args.source, args.destination)
    except (OSError, ValueError, plistlib.InvalidFileException, ExpatError) as error:
        parser.exit(1, f"Unable to prepare Onegram metadata: {error}\n")
    print("Prepared SBAppUsesLocalNotifications=true; original input preserved")


if __name__ == "__main__":
    main()
