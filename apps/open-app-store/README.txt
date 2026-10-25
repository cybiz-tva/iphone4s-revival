Open App Store — iPhone 4s / iOS 9

Scope
A curated single-app catalog, source directory, installed-app launcher, and
direct IPA installer for selected official compatible releases. It is NOT an
exhaustive list of every public app and cannot make modern iOS/ARM64 apps work.
Apple/Cydia-only entries are labeled handoffs. Their account, purchase, package
dependency, and storage rules still apply.

Built-in catalog: 13 app entries, 18 source/reference entries.
Installed source cache adds 42 developer-repository packages plus the user-added
AltStore listings; current device catalog totals 61 entries. Package entries can
be apps, tweaks, libraries or command-line tools, not exclusively GUI apps.
Direct-install releases: iNDS 1.10.8 and ScummVM 2.2.0.
Both official IPAs were downloaded and inspected for ARMv7, MinimumOSVersion,
bundle ID, size, and SHA-256. ScummVM matches its developer-published checksum.
The store is installed with installation capability enabled. The user confirmed
Apps and Sources render correctly and Onegram shows Open. Native UI diagnostics
confirmed the original catalog and the local command bridge; see repository
import verification below for the expanded catalog.
Final on-device download-only checks passed for both iNDS and ScummVM, including
HTTPS, pinned hashes, archive metadata and executable compatibility.
The actual installation backend successfully installed iNDS under the user data
partition. ScummVM passed native archive/executable validation; its installation
has not been tested. Seven catalog policy tests and five on-device ZIP guard
cases passed (valid, traversal, duplicate, encryption, expanded-size bounds).
Apple and Cydia entries use clearly labeled official-store handoffs, as requested.

Privileges and storage
The installed GUI runs as mobile, with application-identifier
dev.codex.OpenAppStore and the private InstallForLaunchServices entitlement.
It uses the existing jailbreak signing support. No root daemon, setuid shell,
arbitrary URL installer, or global trust-policy change is added.
The app lives at /var/local/open-app-store/OpenAppStore.app on the data partition.
Downloads are under /var/mobile/Library/OpenAppStore/Downloads.
Only this new app is explicitly registered; no global icon cache refresh.

Controls
Every GUI install requires a native confirmation. Only bundled allowlisted
entries with pinned hashes may reach the installer. Existing apps are opened,
not replaced, with another installed-state check immediately before installation.
This preserves the customized Onegram installation and its notification helper.
Remote websites use an isolated SFSafariViewController; untrusted remote HTML is
never loaded into the local UIWebView with the native command bridge. The custom
oas navigation scheme is NOT registered with iOS for external callers.

Validation before installation
HTTPS and approved final download host; expected byte count; pinned SHA-256;
safe ZIP paths, no symlinks, bounded entry count and expanded size; one main app;
expected bundle ID; iPhone family and minimum iOS; compatible ARMv7 executable;
no encrypted executable; compatible Mach-O deployment target.
The OS performs actual IPA extraction and installation. The result must have a
registered path under /var or /private/var before success is shown.

Privacy
No Apple ID, password, Telegram session, or user message access is needed.
Download sessions do not use cookie or credential stores. No analytics, remote
catalog scripts, or automatic app updates. Catalog descriptions and URLs are
bundled read-only. Official hosting is not a guarantee of bug-free software or
working online services. Historical/modern repositories are labeled accordingly.

Build
sh build.sh — tested compiler family: Apple clang 15 / classic ARMv7 linker.
The stubs contain linker interfaces, not redistributed Apple implementation code.
No third-party app IPAs are bundled in the store or this source directory.

Sources checked 25 October 2026:
https://github.com/iNDS-Team/iNDS/releases
https://downloads.scummvm.org/frs/scummvm/2.2.0/
https://github.com/IllyaGIF/Onegram/releases
https://www.tigisoftware.com/download/filza.php
https://download.videolan.org/pub/videolan/vlc-iOS/
https://buildbot.libretro.com/stable/1.9.0/apple/ios/
https://mirrors.kodi.tv/releases/darwin/ios/
https://cydia.ppsspp.org/
https://cydia.saurik.com/
https://chariz.com/
https://havoc.app/
See app/catalog.json for all source notes and legacy/unreachable endpoints.

Use
Open the green Open App Store icon. Use Install here for direct IPA entries,
Installed to launch recognized apps, and Sources for official developer links
and repository notes. Every direct installation asks for confirmation. Existing
apps display Open and are not replaced. The catalog is a curated starting set;
source browsing does not imply that every linked release supports this phone.

Verification does not establish that a discontinued app's online service works.
No games or ROMs are included. This source snapshot is published in
cybiz-tva/iphone4s-revival; compiled apps and device source caches are excluded.

Added catalog sources
Open Sources → Add source and paste a complete HTTPS JSON catalog URL.
Supports an apps array in AltStore-style catalogs and Open App Store catalogs.
Added listings appear in Apps → Source apps and in search. Each added source
has Refresh and Remove controls. Sources persist across app launches in
/var/mobile/Library/OpenAppStore/sources.json on the data partition.
Removing a source removes only its listings, never installed apps.

Imported JSON listings are source-provided and unreviewed. The latest listed version
and declared minimum iOS are shown; no ARMv7/service compatibility is inferred.
These listings open HTTPS source/website links, not the privileged installer.
Installable built-in entries remain separate and pinned. Imported bundle IDs,
installer metadata, scripts, images and handoff URLs are not trusted or imported.
Ordinary HTML pages are not app catalogs. Cydia/APT support was added below.
Limits: 20 sources, 1,000 entries per source, 2 MB JSON accepted after download.
Source updates are manual; no background polling. Cookies and saved credentials
are not used. Remote catalogs are parsed as JSON and never executed as HTML/JS.

Source parser tests: tests/native-source.c (five fixture cases, HTTPS checks,
metadata stripping and normalized-data roundtrip). Runs on ARMv7 device without
installer entitlements. tests/native-source-fetch.c exercises the live downloader
without saving sources or installing apps.
Catalog schema reference: https://faq.altstore.io/developers/make-a-source

Cydia repository indexing (25 October 2026)
Sources → Add source now accepts an HTTPS flat Cydia repository URL, or a full
Packages.bz2 / Packages.gz / Packages index URL, as well as JSON catalogs.
The importer checks the provided URL and then Packages.bz2, Packages.gz and
Packages below it. It supports bzip2/gzip indexes using bounded libarchive
decompression (8 MB compressed / 16 MB expanded, at most 1,000 listings).
Only iphoneos-arm and architecture-independent entries are considered. Simple
firmware constraints outside iOS 9.3.6 and explicitly named 64-bit builds are
omitted; alternative dependencies and actual binaries need Cydia/user review.
Duplicate package IDs are reduced to one listing using numeric version ordering;
Cydia remains responsible for Debian version selection and full dependencies.
This is candidate filtering, not an assurance of ARMv7 binary compatibility.

Source cards offer List repo packages, View apps, Add to Cydia, Refresh and Remove
as appropriate. Repo listings have Cydia buttons built from validated package IDs.
These are official-store handoffs, not a new root package installer. Cydia may
write system files; the store cannot guarantee data-only package installation.
Repo URLs still need to be added in Cydia (use Add to Cydia and review there).
Known existing customized apps, including Onegram, use Open when their package
ID matches a bundled, trusted application identifier. Their files are untouched.

Indexed developer repositories on this phone:
TIGI Software: 3 listings — https://tigisoftware.com/cydia/
IllyaGIF: 2 listings — https://illyagif.github.io/repo/
Computershik: 3 listings — https://computershik73.github.io/repo/beta/
LukeZGD: 23 listings — https://lukezgd.github.io/repo/
InvoxiPlayGames: 11 listings — https://cydia.invoxiplaygames.uk/
No packages from these indexes were installed by adding their catalogs.
Existing JSON sources were preserved, with a pre-import source-cache backup
in the app's data directory. No Cydia source list, Onegram registration, global
icon cache or notification helper was modified.

Validation: on-device live HTTPS imports passed for TIGI, Computershik and LukeZGD.
All five downloaded indexes passed native decompression/parser normalization.
Native repository tests cover architecture and firmware exclusions, invalid
package IDs, duplicate versions and Cydia-only handoff generation.

Host-only build and policy checks
sh build.sh
python3 -m unittest discover -s tests
node --check app/app.js
The build produces build/OpenAppStore and build/register-store locally; neither
is committed. Building does not connect to or modify a phone.

Deployment status and scope
This is an experimental source snapshot, not a packaged general-purpose installer.
Installation on the case-study phone was scoped to its new bundle path, signed
with installer-entitlements.plist using its already installed signing support.
The temporary registration tool used registration-entitlements.plist, registered
only dev.codex.OpenAppStore, and was removed afterward. Updates replaced only
this app's files and opened its existing registration. No global uicache command
was used. The tool's --open option only launches the existing store.
Do not use broad registration/cache resets on the case-study phone: they previously
made relocated apps disappear. Existing source caches remain local and aren't
part of this repository. Installing this snapshot on other phones is untested.
