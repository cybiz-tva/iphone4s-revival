"""Build-time policy gate for the bundled, manually reviewed catalog."""
import json
import re
from pathlib import Path
from urllib.parse import urlsplit
APPROVED = {'inds': ('https://github.com/iNDS-Team/iNDS/releases/download/', {'github.com', 'release-assets.githubusercontent.com', 'objects.githubusercontent.com'}), 'scummvm': ('https://downloads.scummvm.org/frs/scummvm/', {'downloads.scummvm.org'})}
PROTECTED = {'dev.IllyaGIF.Onegram', 'com.saurik.Cydia', 'com.tigisoftware.Filza', 'com.officialscheduler.mterminal'}

def require(condition, message):
    if not condition:
        raise ValueError(message)

def validate(data):
    source_ids = [source['id'] for source in data['sources']]
    require(len(source_ids) == len(set(source_ids)), 'Duplicate source ID')
    app_ids = [app['id'] for app in data['apps']]
    require(len(app_ids) == len(set(app_ids)), 'Duplicate app ID')
    for source in data['sources']:
        require(re.fullmatch('[a-z0-9-]+', source['id']), 'Catalog policy rejected an entry')
        require(urlsplit(source['url']).scheme == 'https', 'Catalog policy rejected an entry')
    for app in data['apps']:
        require(re.fullmatch('[a-z0-9-]+', app['id']), 'Catalog policy rejected an entry')
        require(app['source'] in source_ids, 'Catalog policy rejected an entry')
        require(re.fullmatch('#[a-fA-F0-9]{6}', app['color']), 'Catalog policy rejected an entry')
        require(app['kind'] in {'ipa', 'managed', 'cydia', 'apple', 'reference'}, 'Catalog policy rejected an entry')
        if app['kind'] == 'ipa':
            require(app['bundle'] not in PROTECTED, 'Protected customized app')
            prefix, hosts = APPROVED[app['source']]
            parsed = urlsplit(app['url'])
            require(parsed.scheme == 'https' and (not parsed.username) and (not parsed.password), 'Catalog policy rejected an entry')
            require(app['url'].startswith(prefix) and (not parsed.query) and (not parsed.fragment), 'Catalog policy rejected an entry')
            require(parsed.hostname in hosts and set(app['hosts']) == hosts, 'Catalog policy rejected an entry')
            require(re.fullmatch('[a-f0-9]{64}', app['sha256']), 'Catalog policy rejected an entry')
            require(0 < app['bytes'] <= 800 * 1024 * 1024, 'Catalog policy rejected an entry')
            version = tuple(map(int, app['minOS'].split('.')))
            require(version <= (9, 3, 6), 'Catalog policy rejected an entry')
        elif app['kind'] in {'cydia', 'apple'}:
            require(urlsplit(app['handoff']).scheme == ('cydia' if app['kind'] == 'cydia' else 'itms-apps'), 'Catalog policy rejected an entry')
    return data
if __name__ == '__main__':
    catalog = validate(json.loads((Path(__file__).parent / 'app/catalog.json').read_text()))
    print(f"Catalog policy passed: {len(catalog['apps'])} apps, {len(catalog['sources'])} sources")
