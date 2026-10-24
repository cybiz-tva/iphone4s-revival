# Session history: failures as well as successes

## 1. Carrier activation was the original problem

The phone showed **SIM Not Supported** with Airtel India and with du and Etisalat SIMs from the UAE. A third-party carrier report identified **Australia Telstra / SIMLock Locked**. Its sales region was Australia/New Zealand; the sales model alone was not treated as conclusive proof of its carrier.

3uTools displayed **activation error -1106**. That error was not decoded into a universal cause. The unsupported-SIM message and carrier report were the relevant evidence in this case. A Telstra complaint was submitted; the complaint reference and report identifiers are deliberately omitted here. No free software carrier unlock was established. Neither the later jailbreak nor the Apple Account fix changed the carrier lock.

## 2. Legacy app installation exposed an account problem

The same Apple Account could work on a Mac but fail on the phone under **Settings → iTunes & App Store** and in the App Store. The phone displayed **Verification Failed — There was an error connecting to the Apple ID server** even with intentionally incorrect credentials. That behavior suggested failure before ordinary credential validation, but it did not by itself identify the cause.

Separate desktop tools produced HTTP **429 Too Many Requests** at a GrandSlam endpoint and HTTP **301 Moved Permanently** at an old iTunes authentication URL. Those were desktop-tool responses, not proof that the phone used a wrong endpoint. Date/time checks and repeated sign-in attempts did not resolve the phone problem.

## 3. Backup and jailbreak

A backup was made before jailbreaking. A verified Phœnix 6 IPA was tried through Sideloadly, but developer verification failed. Carbon subsequently worked; the user reported that Cydia appeared. StoreAuthFix 1.4 was installed and SpringBoard restarted, but sign-in still failed.

## 4. USB diagnostics isolated provisioning

OpenSSH was installed. USB SSH allowed inspection without posting passwords or private logs publicly. No configuration profiles were found. A TLS capture showed a completed native TLS connection to Apple's authentication service. This was not established as a certificate-validation failure.

Targeted AuthKit logging was enabled only for individual attempts with the user's permission and restored afterward. The useful sequence was lookup success, provisioning start success, and provisioning finish failure. See the [authentication chapter](03-apple-account.md).

## 5. Two experiments failed

An ARMv7 tweak that changed only the provisioning client-info header loaded and produced HTTP 401 at finish, instead of the earlier 503. It did not restore sign-in. A second transport-header experiment also did not restore sign-in. Both were removed along with their enable markers. They are documented as failed tests, not offered as fixes.

## 6. 4SGSAFix succeeded

The developer-provided 4SGSAFix 1.0 package was checked and installed. The user confirmed App Store sign-in worked. iCloud initially kept loading; local diagnostic summaries showed successful credential validation, and the user later confirmed iCloud worked too. There was no need to keep the failed experimental patches.

## 7. SSH terminal installed

Modern terminal clients could not simply be installed on iOS 9. MTerminal 1.4-5 was found in the phone's existing BigBoss package metadata. The downloaded package matched its recorded SHA-256, installed successfully, and registered. OpenSSH 6.7p1 was already available.

An outgoing SSH test reached the requested server's TCP port 22 but timed out during banner exchange; the user then reported already using the terminal connection. This repository does not publish their server address or treat the diagnostic attempt as an authenticated session.

## 8. Social app investigation and Onegram repair

WhatsApp's supported minimum was above iOS 9. Opening VLC's compatible App Store listing was not counted as a completed installation. Telegram's current official client was also unsupported on iOS 9, so community clients were investigated. Ninegram's repository described a mock backend, so it was not mistaken for a functioning Telegram app.

Onegram 1.4.2 had an actual upstream package for older ARM devices. Its checksum matched. A normal `dpkg -i` failed because the small system partition ran out of space, despite about 12 GB free on `/var`.

Extracting the app onto `/var` and linking its directory into `/Applications` did not produce an icon. Creating a real app directory with individually linked contents also did not solve it. A first file-by-file repair was too slow and was stopped. The final repair unpacked a complete physical app bundle with ARMv7 executable slices, retaining the large emoji pack on `/var`, rebuilt the app cache, and restarted SpringBoard.

Two iPhones were connected during the repair. USB forwarding was changed to target the 4s's UDID explicitly, rather than relying on device selection order.

**Stopping point:** iOS recognized Onegram and its real executable was running. No Telegram login, messages, calls, or push notifications had been verified yet.
