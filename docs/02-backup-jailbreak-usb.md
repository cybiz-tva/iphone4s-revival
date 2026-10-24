# Backup, jailbreak, and USB access

## Backup before modifying the phone

The Mac used libimobiledevice tools. An example with redacted device selection is:

```sh
idevice_id -l
idevicebackup2 -u YOUR_DEVICE_UDID backup --full /path/to/private-backup
```

Unlock the phone and trust the Mac when requested. Keep the backup outside this repository.

In this session the backup tool reported success and its snapshot state was finished. The result was small: roughly 7.5–8 MB and 194 data files, with 608 index records. The metadata field `IsFullBackup` was false despite requesting `--full`. It was **unencrypted** and did not contain a restorable archive of all installed application IPAs. These observations limit what can honestly be claimed: a backup completed, but it was not a verified complete disk image or a tested recovery of all app data.

## What jailbreak path actually worked

The official Phœnix 6 package was checked before being sideloaded. Developer verification failed on the phone; that attempt was not the successful jailbreak.

Carbon was then used and Cydia appeared. The session used `http://carbon.sep.lol`; HTTPS had a hostname/certificate problem and no TLS warning was bypassed. This is a historical observation, **not** a recommendation to trust an unverified HTTP download in the future. Re-check the project's current provenance and firmware support before using any jailbreak. A generic jailbreak page name is not enough to authenticate a payload.

This guide does not reconstruct unrecorded exploit-button steps or claim a custom IPSW was used. No optional untether was installed by this workflow. After a reboot, jailbreak-dependent apps and services may require reactivating the jailbreak. Do not promise a reset or restore will preserve this setup.

## OpenSSH over a device-specific USB tunnel

OpenSSH was installed from Cydia. On the Mac, forward a local port to device port 22:

```sh
iproxy -u YOUR_DEVICE_UDID -s 127.0.0.1 2223:22
```

Leave that process running. In another Mac terminal:

```sh
ssh -p 2223 root@127.0.0.1
```

The explicit UDID matters when another iPhone is attached. An ambiguous tunnel initially connected/reset incorrectly during the repair; a device-specific tunnel fixed access.

Verify the phone's host key before accepting it and pin it for subsequent connections. If the same phone is mapped to a different local port, verify that its fingerprint still matches rather than silently accepting a new identity. The automation in this case retained the pinned key.

Passwords were entered locally; do not hard-code them in published scripts. Set non-default passwords for privileged device accounts with `passwd` and, where supported, `passwd mobile`. The working session still had a default OpenSSH credential; this guide does **not** claim it was changed during the recorded repair. Do not expose that service to an untrusted network.

The phone lacked several familiar Unix tools (`apt-cache`, Python, `plutil`, and others). Diagnostics used SSH/SFTP and parsed plist files on the Mac. A missing shell utility is not proof of filesystem damage.

## Mac Python issue encountered

Homebrew Python had an `expat` dynamic-library mismatch. The local workaround was:

```sh
DYLD_LIBRARY_PATH=/opt/homebrew/opt/expat/lib /path/to/venv/bin/python3.12 script.py
```

This was a Mac runtime issue, unrelated to iPhone TLS or Apple Account credentials. It is not a required environment variable for every reader.
