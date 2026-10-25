# Bringing an iPhone 4s back into use in 2026

A practical case study of an **iPhone 4s (A1387, iPhone4,1), 16 GB, running iOS 9.3.6 (13G37)**: backup, jailbreak, diagnosing Apple Account authentication, restoring App Store/iCloud sign-in, installing an SSH terminal, and getting a community Telegram client registered and running.

Recorded on **24–25 October 2026**. This documents one device and account session. It does not promise that every account, jailbreak, package revision, or server will behave identically.

## What actually worked

| Goal | Result at the end of this session |
| --- | --- |
| Back up the phone before modifications | Backup tool reported success; important limitations below |
| Jailbreak | Carbon worked and Cydia appeared |
| App Store Apple Account sign-in | User confirmed success after installing 4SGSAFix |
| iCloud sign-in | User subsequently confirmed success |
| SSH terminal | MTerminal 1.4-5 installed; OpenSSH client available |
| Onegram installation | Icon and existing account confirmed working; app later relocated to the data partition |
| Telegram account login / messages | User confirmed the account opens and incoming messages work |
| Onegram notifications outside the app / while locked | Working with the declaration repair and app-specific background helper; user confirmed delivery after 20 minutes |
| Carrier unlock | **Not achieved**; separate Telstra request pending at the time |
| WhatsApp | No verified working iOS 9 solution installed |
| VLC | App Store listing opened; installation not confirmed |

The initial session established Onegram registration and launch. The later follow-up confirmed its icon, account and messages, then fixed system notifications on Home Screen and the lock screen. See the [notification solution](docs/07-onegram-notifications.md) for the tested configuration, source, limits and rollback. A twenty-minute result does not establish overnight reliability or measured battery impact.

## Read in order

1. [Full session chronology](docs/01-session-history.md)
2. [Backup, jailbreak, and USB SSH](docs/02-backup-jailbreak-usb.md)
3. [Apple Account diagnosis and working fix](docs/03-apple-account.md)
4. [MTerminal and connecting to Linux](docs/04-terminal.md)
5. [Onegram: disk space, missing icon, and final repair](docs/05-onegram.md)
6. [Package versions, checksums, and sources](docs/06-packages.md)
7. [Onegram notifications: Home Screen, lock screen, and persistent helper](docs/07-onegram-notifications.md)

The guide retains failed attempts because they explain why the final process took this shape. Follow the successful route first; the experimental provisioning patches did not restore sign-in and were removed.

## Repository contents and privacy

Only original documentation, helper source and control files, tests, and public package hashes belong here. No device backups, private account identifiers, IMEI/UDID/serial numbers, personal server addresses, passwords, authentication captures, signing keys, compiled helper binaries, or third-party binaries are included. Examples use placeholders or a local USB tunnel.

The helper [prepare_onegram.py](scripts/prepare_onegram.py) prepares the final app layout locally from the exact upstream package. It **does not connect to, modify, or authenticate to a phone**. Phone installation commands are explicit in the guide.

The [notification helper](helpers/onegram-notifications/) is different: its device-side components run with private background-execution entitlements and a root LaunchDaemon, with a hook restricted to Onegram. It can increase battery consumption. Building its source on the host does not install it; privileged signing, scoped registration and deployment are explicit in the [notification guide](docs/07-onegram-notifications.md). It does not relaunch a force-quit app and does not provide genuine APNs push.

A jailbreak expands device access and is tied to its supported firmware. Keep a backup and secure OpenSSH before putting the device on an untrusted network. This guide does not disable TLS validation or turn an account authentication fix into a carrier unlock.

## Credits

Credit for the actual jailbreaks, tweaks, and apps belongs to their developers: Carbon, Iocaste, Phœnix, StoreAuthFix, sdhEmily's 4SGSAFix, lordscotland's MTerminal, the OpenSSH project, Cydia Substrate, and IllyaGIF's Onegram and its upstream contributors. The original app-specific helper source here records this integration; it does not claim authorship of those projects. CokePokes' libnotifications documented the local-notification declaration used in the repair; that iOS 10+ library was not installed on this iOS 9 device.

Original material in this repository is MIT licensed. Third-party software remains under its own license and is linked, not redistributed.
