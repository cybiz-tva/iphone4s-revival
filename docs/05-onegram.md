# Onegram: from failed installation to a registered, running app

## Exact package

[Onegram](https://github.com/IllyaGIF/Onegram) is an unofficial Telegram client. The developer documents older iOS and ARM device support. This case used:

```text
Package: dev.illyagif.onegram
Version: 1.4.2-144
Architecture: iphoneos-arm
App bundle ID: dev.IllyaGIF.Onegram
App display name: Onegram
Launcher: Telegram
Real executable: Telegram.real
```

Do not confuse this package with the `iphoneos-arm64` package for newer jailbreaks. Compatibility metadata is not proof of working Telegram login; the latter was still untested at the end of this case.

Download from the developer's repository:

```sh
curl -fL https://illyagif.github.io/repo/debs/onegram_1.4.2-144_iphoneos-arm.deb -o onegram.deb
shasum -a 256 onegram.deb
```

Expected SHA-256:

```text
5b4256ad35f9a1354fe11a826946910c7cbb82c98d67c26618320c534b8db65f
```

The upstream GitHub IPA release has a **different** checksum from the Debian package. Do not compare one artifact against the other's digest.

## Normal install: why it failed here

The first root-shell attempt was:

```sh
dpkg -i /var/tmp/onegram.deb
```

It failed writing `Telegram.real.dpkg-new`: **No space left on device**. Storage was approximately:

| Partition | Size | Free at inspection |
| --- | --- | --- |
| System `/` | 2.3 GB | 103 MB |
| Data `/var` | 13 GB | 12 GB |

The full multi-architecture `Telegram.real` was 114,307,184 bytes. Its ARMv7 slice was 38,852,256 bytes. The emoji pack was another 40,902,711 bytes. Free space on `/var` does not automatically make `/Applications` large enough.

`dpkg` failed, so the final app was **not** successfully installed as a normal package-managed Onegram package. `dpkg-query` initially showed the package name with an empty version. The final repair was a manual app deployment, described below.

## Approaches that did not produce an icon

1. Extract the complete package to `/var/mobile/LegacyApps/Onegram` and link `/Applications/Onegram.app` to its app directory.
2. Replace that directory shortcut with a physical directory, copying `Info.plist` and linking individual contents back to `/var`.
3. Refresh `uicache` and request `uiopen telegram://`.

Those commands returned without a useful error, but the user still saw no icon in Home Screen or search, and no app process was observed. A zero exit code from `uiopen` or `uicache` was insufficient evidence.

A subsequent file-by-file copy over SFTP was too slow and was stopped. The final repair used one compressed archive instead. We did not establish a universal root cause for every missing icon; the complete physical layout plus cache refresh and SpringBoard restart is what changed the result in this case.

## Final successful layout

Keep the complete original package extracted on the **data partition**:

```text
/var/mobile/LegacyApps/Onegram/Applications/Onegram.app/
```

Use a **physical app directory** on the system partition, containing the ordinary resources and ARMv7 versions of its fat Mach-O binaries:

```text
/Applications/Onegram.app/
```

Keep just the large emoji pack linked to its data-partition copy:

```text
/Applications/Onegram.app/OnegramEmoji.pack
  -> /var/mobile/LegacyApps/Onegram/Applications/Onegram.app/OnegramEmoji.pack
```

The [helper](../scripts/prepare_onegram.py) reproduces the local repacking operation. It verifies the exact input package hash, selects the ARMv7 slice of fat Mach-O files, preserves file modes, omits the emoji pack, and writes an archive. It does not redistribute or download the original binary.

On the Mac:

```sh
python3 scripts/prepare_onegram.py /path/to/onegram.deb /path/to/Onegram-armv7.tar.gz
```

The case's generated compressed archive was about 30 MB. Compressed bytes can differ across Python/zlib versions; validate its contents, not an invented universal archive hash.

Transfer **both** the original `.deb` and the generated archive to `/var/tmp` on the phone using your verified USB SSH connection, for example:

```sh
scp -P 2223 onegram.deb Onegram-armv7.tar.gz root@127.0.0.1:/var/tmp/
```

The following are **root-shell commands on the phone**, not the Mac. They replace only the Onegram app directory; do not substitute a broader `/Applications` path. If Onegram already has personal data or a different layout, inspect and back it up first. This exact repair was used on a fresh, failed installation.

```sh
set -e
mkdir -p /var/mobile/LegacyApps/Onegram
dpkg-deb -x /var/tmp/onegram.deb /var/mobile/LegacyApps/Onegram
chown -R mobile:mobile /var/mobile/LegacyApps/Onegram

# Rebuild only this app from the verified ARMv7 archive.
rm -rf /Applications/Onegram.app
tar -xzf /var/tmp/Onegram-armv7.tar.gz -C /
ln -s /var/mobile/LegacyApps/Onegram/Applications/Onegram.app/OnegramEmoji.pack /Applications/Onegram.app/OnegramEmoji.pack

df -h /
uicache
killall SpringBoard
```

`killall SpringBoard` briefly closes the current app and restarts Home Screen. It is not a full phone reboot. The final observed system free space was approximately **42 MB**: this workaround fit, but left little headroom. Monitor space and do not casually install more system-partition packages.

## Verify registration and launch separately

The Mac queried `InstallationProxyService.get_apps` using pymobiledevice3, explicitly selecting the 4s's UDID. The sanitized result was:

```text
dev.IllyaGIF.Onegram
  CFBundleDisplayName: Onegram
  Path: /Applications/Onegram.app
  ApplicationType: System
  CFBundleExecutable: Telegram
Onegram registered: True
```

A phone-shell launch request was then followed by a process check:

```sh
su mobile -c 'uiopen telegram://'
ps ax | grep '[T]elegram'
```

The process list contained:

```text
/Applications/Onegram.app/Telegram.real
```

This establishes registration and a running executable at the check, not successful Telegram account authentication. The user was asked to open Onegram and sign in directly on the phone. **This repository stops before that test.**

Push notifications were not configured. The upstream release mentions a separate push helper; it was not installed in this session.

## Removing the manually deployed app

Close Onegram first. Preserve any account data you want before removal. The app bundle can be removed by deleting exactly `/Applications/Onegram.app`, refreshing `uicache`, and restarting SpringBoard. The retained original files under `/var/mobile/LegacyApps/Onegram` can be removed separately only after confirming they are no longer needed. Removing those original files first would break the emoji-pack link.

This removal was not executed on the working phone. A normal package-manager upgrade may restore the original multi-architecture files and reproduce the storage problem.
