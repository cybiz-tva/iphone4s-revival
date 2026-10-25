import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("catalog_policy", ROOT / "validate_catalog.py")
POLICY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(POLICY)


class CatalogPolicyTests(unittest.TestCase):
    def setUp(self):
        self.catalog = json.loads((ROOT / "app/catalog.json").read_text())

    def test_reviewed_catalog(self):
        POLICY.validate(self.catalog)

    def test_rejects_http_download(self):
        self.catalog["apps"][0]["url"] = self.catalog["apps"][0]["url"].replace("https:", "http:")
        with self.assertRaises(ValueError): POLICY.validate(self.catalog)

    def test_rejects_unknown_redirect_host(self):
        self.catalog["apps"][0]["hosts"].append("example.invalid")
        with self.assertRaises(ValueError): POLICY.validate(self.catalog)

    def test_rejects_protected_app_replacement(self):
        self.catalog["apps"][0]["bundle"] = "dev.IllyaGIF.Onegram"
        with self.assertRaises(ValueError): POLICY.validate(self.catalog)

    def test_rejects_missing_pin(self):
        self.catalog["apps"][0]["sha256"] = ""
        with self.assertRaises(ValueError): POLICY.validate(self.catalog)

    def test_rejects_newer_ios(self):
        self.catalog["apps"][0]["minOS"] = "12.0"
        with self.assertRaises(ValueError): POLICY.validate(self.catalog)

    def test_rejects_duplicate_ids(self):
        self.catalog["apps"].append(copy.deepcopy(self.catalog["apps"][0]))
        with self.assertRaises(ValueError): POLICY.validate(self.catalog)


if __name__ == "__main__": unittest.main()
