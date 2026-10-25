import importlib.util
import plistlib
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / "helpers/onegram-notifications/prepare_onegram_info.py"
SPEC = importlib.util.spec_from_file_location("onegram_info", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class OnegramInfoTests(unittest.TestCase):
    def test_preserves_original_other_fields_and_format(self):
        for fmt in (plistlib.FMT_XML, plistlib.FMT_BINARY):
            with self.subTest(fmt=fmt), tempfile.TemporaryDirectory() as directory:
                source, output = Path(directory) / "original", Path(directory) / "prepared"
                info = {"CFBundleIdentifier": MODULE.BUNDLE_ID, "CFBundleExecutable": "Telegram", "nested": {"preserved": [1, 2]}}
                raw = plistlib.dumps(info, fmt=fmt)
                source.write_bytes(raw)
                MODULE.prepare(source, output)
                self.assertEqual(source.read_bytes(), raw)
                self.assertEqual(plistlib.loads(output.read_bytes()), {**info, "SBAppUsesLocalNotifications": True})
                self.assertEqual(output.read_bytes().startswith(b"bplist00"), fmt == plistlib.FMT_BINARY)

    def test_rejects_another_app_without_creating_output(self):
        with tempfile.TemporaryDirectory() as directory:
            source, output = Path(directory) / "original", Path(directory) / "prepared"
            source.write_bytes(plistlib.dumps({"CFBundleIdentifier": "example.other"}))
            with self.assertRaises(ValueError):
                MODULE.prepare(source, output)
            self.assertFalse(output.exists())

    def test_rejects_in_place_edit(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "original"
            source.write_bytes(plistlib.dumps({"CFBundleIdentifier": MODULE.BUNDLE_ID}))
            original = source.read_bytes()
            with self.assertRaises(ValueError):
                MODULE.prepare(source, source)
            self.assertEqual(source.read_bytes(), original)

    def test_does_not_overwrite_existing_destination(self):
        with tempfile.TemporaryDirectory() as directory:
            source, output = Path(directory) / "original", Path(directory) / "prepared"
            source.write_bytes(plistlib.dumps({"CFBundleIdentifier": MODULE.BUNDLE_ID}))
            output.write_bytes(b"existing backup")
            with self.assertRaises(FileExistsError):
                MODULE.prepare(source, output)
            self.assertEqual(output.read_bytes(), b"existing backup")


if __name__ == "__main__":
    unittest.main()
