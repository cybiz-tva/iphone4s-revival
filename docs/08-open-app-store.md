# Open App Store: app catalog and repository browsing

Recorded 25 October 2026 on one jailbroken iPhone 4s, iOS 9.3.6 / ARMv7.

## Result

The original native application **Open App Store** is installed and runs as the mobile user. Its app bundle is on the data partition at `/var/local/open-app-store/OpenAppStore.app`; downloads and added-source metadata are under `/var/mobile/Library/OpenAppStore/`.

The built-in catalog contains 13 app entries and 18 source references. Five imported developer repositories supplied 42 package listings; an existing user-added AltStore catalog supplied six more. The observed UI rendered 61 total app/package entries. Those counts describe that device snapshot, not a universal app archive or 61 verified working GUI apps.

The user confirmed the initial Apps/Sources interface and Onegram's Open button. Native UI diagnostics confirmed the expanded catalog and local navigation bridge after repository import.

## Installation choices

- **Install here:** two selected, developer-hosted legacy IPAs: iNDS 1.10.8 and ScummVM 2.2.0. Both passed native HTTPS, exact size, pinned SHA-256, archive, bundle, executable and iOS compatibility checks. ScummVM's hash also matched the publisher's checksum. The actual installation backend installed iNDS on the data partition. ScummVM installation has not been tested.
- **Apple / Cydia:** labeled handoffs to those official clients. Their authorization, dependencies, purchases and package storage rules still apply. Cydia packages may modify system files; data-only installation is not guaranteed for those handoffs.
- **Source:** imported JSON listings and reference entries open their source/website. Importing a JSON catalog does not grant it direct installation privileges.
- **Open:** recognizes existing app registrations and preserves them instead of replacing them. A trusted built-in bundle-ID mapping protects the customized Onegram entry when its matching repository package is listed.

No third-party apps were installed merely by importing repository listings. iNDS was the separate, successful installation-backend test.

## Add sources that list actual packages

Open **Sources → Add source** and enter an HTTPS JSON catalog or flat Cydia repository URL. Sources persist across launches and have Refresh/Remove controls. **List repo packages** imports a supported reference repo; **View apps** searches its publisher name. **Add to Cydia** hands the source URL to Cydia for review. A repo must also be known to Cydia before Cydia can install its packages.

| Indexed developer source | Candidate listings in this snapshot |
| --- | ---: |
| [TIGI Software](https://tigisoftware.com/cydia/) | 3 |
| [IllyaGIF](https://illyagif.github.io/repo/) | 2 |
| [Computershik](https://computershik73.github.io/repo/beta/) | 3 |
| [LukeZGD](https://lukezgd.github.io/repo/) | 23 |
| [InvoxiPlayGames](https://cydia.invoxiplaygames.uk/) | 11 |

The importer tries the provided URL followed by `Packages.bz2`, `Packages.gz` and `Packages`. It accepts `iphoneos-arm` or architecture-independent packages, excludes straightforward incompatible firmware constraints and explicitly named 64-bit builds, and collapses duplicate package IDs. This is metadata filtering, not proof of actual ARMv7 binary or online-service compatibility. Alternative dependencies and Debian version selection remain Cydia's responsibility. Packages include tweaks, libraries and command-line tools as well as apps.

Modern JSON catalogs can import successfully while listing apps that require newer iOS. The [official AltStore feed](https://apps.altstore.io/) loaded six listings in a native read-only test; that did not establish iOS 9 compatibility.

## Source and validation

See [original source and detailed notes](../apps/open-app-store/README.txt), [source directory](../apps/open-app-store/SOURCES.txt) and [sample JSON catalog](../apps/open-app-store/example-source.json).

Seven host catalog-policy tests passed. Native source tests covered malformed inputs, field stripping, HTTPS restrictions and normalized-data roundtrip. Native repository tests covered architecture/firmware exclusions, invalid package IDs, duplicate versions and Cydia-only handoffs. Five ZIP guard cases passed on-device. Live HTTPS repository imports passed for TIGI, Computershik and LukeZGD; all five downloaded indexes passed native decompression and normalization.

Only built-in allowlisted IPAs can enter the direct installer. Their downloads have pinned hashes, bounded archive inspection, compatible main-executable checks, an installation confirmation, and installed-state checks. Imported catalog/repository metadata supplies no privileged shell or arbitrary IPA installer. Sources are manually trusted; APT signature verification is not implemented by this browser. Remote pages use an isolated Safari view rather than the local UIWebView command bridge.

Limits include 20 added sources, 1,000 entries per source, 2 MB accepted JSON and 8 MB compressed / 16 MB expanded repository data. These are acceptance/decompression limits; the download is checked after completion, not a streaming hard network-transfer cap. Numeric duplicate-version ordering is not a Debian version resolver. Tests are recorded outcomes, not a general security audit.

No global icon-cache rebuild, Onegram registration update, respring, new root daemon or system-wide signing change was performed for this app. Its private installer entitlement uses the signing support already authorized on the case-study phone. The snapshot is experimental and was not installed on other devices.
