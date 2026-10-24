# Apple Account sign-in: the useful evidence and working fix

## Evidence before changing anything

The failure occurred in both Settings and the App Store. A different account was being used on the phone from the account on the Mac. Consequently the Mac's Advanced Data Protection setting could not establish the phone account's configuration. The user reported no physical security keys and no verification prompt during the failed attempts.

Read-only inspection found no installed configuration profiles. A WhatsApp app provisioning profile was not treated as a device-wide configuration profile or blamed for account sign-in.

A native network capture showed a TLS 1.2 session to `gsa.apple.com`, using `ECDHE-RSA-AES256-GCM-SHA384`, with bidirectional application traffic. The connection reached Apple; the generic UI error did not mean the phone could not reach the server. An unrelated certificate issue in location-service traffic was not evidence of the same fault in Apple authentication.

## Targeted logging, then restoration

For individual sign-in attempts, AuthKit file logging was enabled using `AKFileLoggingEnabled` in the mobile user's `.GlobalPreferences.plist`. Each time the original value/absence was recorded, logs stayed local, and that setting was restored afterward. The restored state was checked. The account daemon needed an actual process restart; a non-forced stop did not always stop it in this session.

Sensitive diagnostic locations included:

```text
/var/mobile/Library/Logs/CrashReporter/DiagnosticLogs/Accounts/authkit.log
/var/mobile/Library/Logs/CrashReporter/DiagnosticLogs/Accounts/appleidtraffic.log
```

These logs can contain account/session material. They are deliberately absent from this repository. Do not enable verbose logging indefinitely or upload whole captures to an issue tracker.

The sanitized failure sequence was:

| Step | Observed result |
| --- | --- |
| Local anisette failure | Error -8007 |
| Lookup | HTTP 200 |
| `startMachineProvisioning` | HTTP 200; local ADI start succeeded |
| `finishMachineProvisioning` | HTTP 503 with an HTML response |
| Parsing the unexpected response | NSCocoa error 3840 |

A native activation record was present. That did not establish that a different SIM would be accepted or that the carrier lock was removed.

## Attempts that did not restore login

- **StoreAuthFix 1.4:** installed and loaded, but sign-in still failed. It remained installed when the later fix worked, so this case does not prove it is unnecessary.
- **Client-info experiment:** changed only `X-Mme-Client-Info` on the HTTPS GrandSlam provisioning start/finish pair to a Windows-style profile. Start returned 200 and finish 401. It failed and was removed.
- **Transport-header experiment:** retained native identity and changed content type, connection handling, and client timestamp. The user still saw Verification Failed; no useful targeted request observation established a successful change. It was removed.

Both experiments were limited to `akd`, gated, and temporary. Neither disabled TLS validation, changed activation tickets, nor repaired the carrier lock. Their code is not included as an endorsed repair.

## The package that changed the result

Developer repository: [repo.sdh.gay](https://repo.sdh.gay/).

Package: `gay.sdh.4sgsafix`, version **1.0**, ARM, firmware **iOS 9.x**. The downloaded package matched the SHA-256 in [the package manifest](../checksums.sha256). Review of its small binary indicated a `MGCopyAnswer` hook that reports `iPod1,1` when the scoped processes ask for `ProductType`. The developer describes this as a workaround for GrandSlam rejecting the iPhone 4s product identifier. That explanation is the developer's claim; the observed result here is successful sign-in after installation.

Install through Cydia after adding the developer source, or download the exact package and verify it before copying it to the phone:

```sh
curl -fL https://repo.sdh.gay/debs/gay.sdh.4sgsafix_1.0_iphoneos-arm.deb -o 4SGSAFix-1.0.deb
shasum -a 256 4SGSAFix-1.0.deb
```

Then in a root shell **on the phone**, after transferring the verified file:

```sh
dpkg -i /var/tmp/4SGSAFix-1.0.deb
killall -9 akd accountsd
killall Preferences
```

Reopen **Settings → iTunes & App Store** and sign in directly on the device. A daemon that was not running may produce a harmless “no matching processes” message; re-opening Settings starts services as needed. Never post your password or verification code in a public issue.

The user confirmed App Store sign-in succeeded. iCloud initially continued loading; diagnostic summaries indicated credential validation had succeeded. Settings was closed/reopened, and the user subsequently confirmed iCloud worked. No claim is made that every iCloud feature, Notes/CloudKit operation, or keychain function was fully tested.

If removing this tweak is necessary, its package-manager removal is `dpkg -r gay.sdh.4sgsafix`, followed by restarting the affected processes. That rollback is a procedure, not a test performed after the successful result. Keep the working tweak installed when preserving this setup.
