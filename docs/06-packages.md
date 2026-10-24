# Versions, sources, and integrity

These are the **exact artifacts observed in this case**, not a promise of the latest versions. Re-check developer pages before any future install. Hash matching confirms that bytes match a recorded artifact; it does not prove all behavior is safe.

| Artifact | Version | Role / observed result |
| --- | --- | --- |
| iOS | 9.3.6 (13G37) | Phone firmware |
| Phœnix | 6 | Verified IPA, but developer verification failed |
| Carbon | Version not captured | Successful jailbreak; Cydia appeared |
| StoreAuthFix | 1.4 | Installed; insufficient alone in this case |
| 4SGSAFix | 1.0 | Installed; user confirmed account sign-in afterward |
| MTerminal | 1.4-5 | Installed; ARMv7 verified |
| OpenSSH | 6.7p1 | SSH client present |
| OpenSSL on phone | 0.9.8zg | Reported by SSH client |
| Onegram Debian package | 1.4.2-144, iphoneos-arm | Verified, manually deployed after storage failure |
| Onegram GitHub release | 1.4.2 | IPA offered upstream; final deployment used `.deb` instead |
| Paramiko on Mac | 3.5.1 | Used with the older phone SSH server |
| Python on Mac | 3.12 | Used for plist/package parsing and USB tooling |
| Sideloadly | 0.71.1 | Used during unsuccessful Phœnix trust route |
| 3uTools | 3.18 | Reported activation failure; did not unlock carrier |

## Recorded SHA-256 digests

See [checksums.sha256](../checksums.sha256). No binaries are bundled here.

- Phœnix 6 IPA: `d7e5e84e4081b6e138d2f3ed5bb68ff944164e7483a9962c32b08b37e57fa161`
- StoreAuthFix 1.4 Debian package: `572962e1565341a937bfe9760ff02c556e1c66527f916a3a5927017a9de1e854`
- 4SGSAFix 1.0: `959b0ffbecae26a76358cdc4efb10d79e97e0898decbc8b2ce62ba9eab91537b`
- MTerminal 1.4-5: `a0138646daa42478b726ec164a261a9c31b8ac29f91e5cf500f194ece9039924`
- Onegram 1.4.2-144 ARM Debian package: `5b4256ad35f9a1354fe11a826946910c7cbb82c98d67c26618320c534b8db65f`
- Onegram 1.4.2 multi-architecture IPA, upstream release digest (not the final installed artifact): `ef4bcd1e82102a400eb5c8d54949853fa6de18daf68cdcc7fa2f1ed585434733`

## Primary project/reference links

- [Phœnix project](https://phoenixpwn.com/)
- [Carbon address used in the session](http://carbon.sep.lol) — historic HTTP source, with the provenance limitation described in the jailbreak chapter
- [4SGSAFix developer package metadata](https://repo.sdh.gay/Packages)
- [MTerminal Cydia package](https://cydia.saurik.com/package/com.officialscheduler.mterminal/)
- [libimobiledevice](https://libimobiledevice.org/)
- [iproxy / libusbmuxd](https://github.com/libimobiledevice/libusbmuxd)
- [pymobiledevice3](https://github.com/doronz88/pymobiledevice3)
- [Onegram source](https://github.com/IllyaGIF/Onegram)
- [Exact Onegram GitHub release](https://github.com/IllyaGIF/Onegram/releases/tag/1.4.2)
- [Onegram developer package index](https://illyagif.github.io/repo/Packages)
- [Telegram supported devices](https://telegram.org/faq#q-which-devices-can-i-use)
- [WhatsApp supported operating systems](https://faq.whatsapp.com/1150261202542208)
- [VideoLAN iOS download/compatibility](https://www.videolan.org/vlc/download-ios.html)
- [Telstra carrier unlock guidance](https://www.telstra.com.au/support/mobiles-devices/unlock-phone-imei)

No carrier unlock, activation ticket, signed Apple software, or Telegram account session is distributed by this repository.
