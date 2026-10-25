# Onegram notifications outside the app and while locked

On **25 October 2026**, the user confirmed real Onegram notifications on Home Screen and on the lock screen, then confirmed they were still working **after 20 minutes** with a persistent helper installed. This was an iPhone 4s running iOS 9.3.6, Onegram 1.4.2, Cydia Substrate, and the existing Carbon jailbreak with an Iocaste untether. The app remained on the data partition.

This is an independent, app-specific **keep-alive workaround**, not the upstream Onegram Push Helper or genuine Apple APNs push. Overnight delivery, battery consumption, and a full reboot were not tested. Automatic helper startup is configured; open Onegram once after reboot and leave it in the app switcher. Swiping the app away stops its connection, and this helper deliberately does not reopen it.

## What fixed it

Two separate issues needed addressing:

1. **System notification eligibility.** The manually deployed system app lacked `SBAppUsesLocalNotifications=true` in `Info.plist`. An in-app banner was not an iOS notification. A separate diagnostic app initially reported notification types `0`; adding the declaration and updating only its registration produced types `7` and a visible lock-screen test. The same declaration was then added to Onegram. Manually enabling its BulletinBoard preferences alone had not produced working notifications.
2. **Background execution and connection lifetime.** Onegram's source pauses networking after its background-task allowance. A root helper holds a `BKSProcessAssertion` only for the running Onegram executable. A Onegram-only Substrate hook preserves its background connection while a fresh, PID-matched helper heartbeat exists. It resumes the connection when the helper becomes available in the background.

The final helper does not read Telegram account credentials, tokens, or message bodies; it does not use a relay. It does not change Wi-Fi policy, global app trust settings, or other apps' background behavior. Its privileged private APIs and continuous connection can increase battery use.

The declaration was documented by [libnotifications](https://github.com/CokePokes/libnotifications). **That library requires iOS 10+ and was not installed.** Its documented declaration was independently tested here on iOS 9. The app lifecycle and networking behavior were checked against the [Onegram source](https://github.com/IllyaGIF/Onegram).

## Preserve the working icon and account

The final physical app path was:

```text
/var/local/relocated-apps/Onegram.app
```

The compatibility link `/Applications/Onegram.app` points there. LaunchServices was explicitly registered with the **physical data-partition path**. The account remained signed in.

**Do not run a global `uicache`, rebuild all application databases, unregister Onegram, or replace its launcher to apply this notification repair.** In this case, a global cache refresh dropped the linked apps' registrations and made their icons disappear. Their files remained intact; explicit registration with the physical paths restored the icons. The successful notification repair updated only Onegram's existing registration, without that global refresh.

The older [installation chapter](05-onegram.md) describes the preceding system-partition layout. Its cache-refresh commands are historical and should not be applied to the linked layout above. If your installed app path differs, review and adapt the fixed path in `register-onegram.c` before building; do not move a working app just to match this example. The keeper's executable allowlist must also match your actual executable path.

## Source and build

Original source and control files are in [helpers/onegram-notifications](../helpers/onegram-notifications/). No Onegram binaries, Apple frameworks, credentials, device identifiers, or compiled helper binaries are distributed.

- `keeper.c`: bounded trial or persistent `--daemon` mode; exact Onegram executable allowlist; never launches an app.
- `network-hook.c`: only hooks `TGTelegramNetworking.pause` in bundle `dev.IllyaGIF.Onegram`; falls back to the original pause if the heartbeat is missing, stale by more than 45 seconds, or belongs to another PID. No synthetic notification hooks remain.
- `register-onegram.c`: updates **only** the fixed Onegram path and refuses a different bundle identifier. It does not unregister the app or clear icon caches.
- `prepare_onegram_info.py`: host-only plist preparation; preserves other fields, the input file, and XML/binary format. It does not connect to a phone.
- `service.plist`, the Substrate filter, entitlements, and enable/disable scripts: the explicit privileged configuration.
- `stubs/libSystem.tbd`: a minimal text linker interface, not an Apple framework implementation. Remaining symbols resolve from frameworks loaded at runtime.

The build was checked with **Apple clang 15.0.0** and its classic linker, which still supports ARMv7. Newer toolchains may remove that target or linker. This is a freestanding build, not a bundled iOS SDK:

```sh
sh helpers/onegram-notifications/build.sh
python3 -m unittest discover -s tests -v
```

Building does not sign, install, connect to, or modify a phone. The output is `keeper`, `OnegramKeepAlive.dylib`, and `register-onegram` beside the sources; Git ignores these binaries.

## Prepare the declaration on the host

Enable Onegram's notification and background-notification options, and use an unmuted chat for testing. The tested app had `TGIOS6BackgroundNotificationsEnabled` enabled. Its APNs option was also enabled, but no APNs entitlement or certificate was added and this result does not establish genuine APNs registration. The helper does not override chat muting or notification preferences; configure those through the app rather than replacing its entire preferences file.

First confirm Onegram's current icon, login, and app path. Keep a backup. Using the already verified [USB SSH connection](02-backup-jailbreak-usb.md), download that app's `Info.plist` to a local working directory, then run:

```sh
python3 helpers/onegram-notifications/prepare_onegram_info.py \
  Info.original.plist Info.patched.plist
```

The input must identify `dev.IllyaGIF.Onegram`. The output must be a new file. The only intended field change is:

```xml
<key>SBAppUsesLocalNotifications</key>
<true/>
```

## Device installation used in this layout

These are **root-shell commands on a jailbroken phone**, not host commands. The device needs `ldid`, Cydia Substrate, and the ability to run locally signed code. The tested phone already had AppSync Unified 116.0; these instructions do not install it or change global signature policy. Review the private entitlements before installing: the keeper receives system-app and unlimited process-assertion rights, and the temporary registration tool receives `InstallForLaunchServices` and MobileContainerManager access. That tool accepts no target argument and is restricted to the fixed Onegram path.

Transfer the three built binaries, the four helper `.plist` files, `enable.sh`, `disable.sh`, and your prepared `Info.patched.plist` into `/var/tmp/onegram-notifications`. Use your trusted transfer method; no SSH password or device-specific connection configuration is included here.

The following is a **first-install recipe for the physical path above**, not an upgrade script. If the helper's standard paths already exist, inspect the existing service instead of overlaying a running installation. It stages and signs helper code before applying the app metadata change:

```sh
set -e
base=/var/local/onegram-keepalive
stage=/var/tmp/onegram-notifications
app=/var/local/relocated-apps/Onegram.app

test -d "$app"
test ! -e /Library/LaunchDaemons/dev.codex.onegram-keepalive.plist
test ! -e /Library/MobileSubstrate/DynamicLibraries/OnegramKeepAlive.dylib
test ! -e /Library/MobileSubstrate/DynamicLibraries/OnegramKeepAlive.plist
test ! -e "$base/Info.before-local-notifications.plist"
test ! -e "$base/enabled"

mkdir -p "$base/logs" "$base/daemon-logs"
chown root:wheel "$base" "$base/daemon-logs"
chmod 755 "$base"
chmod 750 "$base/daemon-logs"
chown mobile:mobile "$base/logs"
chmod 755 "$base/logs"

for name in keeper OnegramKeepAlive.dylib register-onegram entitlements.plist \
  registration-entitlements.plist service.plist OnegramKeepAlive.plist enable.sh disable.sh; do
  cp "$stage/$name" "$base/$name"
  chown root:wheel "$base/$name"
done
chmod 755 "$base/keeper" "$base/register-onegram" "$base/enable.sh" "$base/disable.sh"
chmod 644 "$base/OnegramKeepAlive.dylib" "$base/"*.plist
ldid -S"$base/entitlements.plist" "$base/keeper"
ldid -S "$base/OnegramKeepAlive.dylib"
ldid -S"$base/registration-entitlements.plist" "$base/register-onegram"

cp -p "$app/Info.plist" "$base/Info.before-local-notifications.plist"
chown root:wheel "$base/Info.before-local-notifications.plist"
chmod 600 "$base/Info.before-local-notifications.plist"
cp "$stage/Info.patched.plist" "$app/Info.plist.new"
chown root:wheel "$app/Info.plist.new"
chmod 644 "$app/Info.plist.new"
killall Telegram.real 2>/dev/null || true
mv "$app/Info.plist.new" "$app/Info.plist"
"$base/register-onegram"

ln -s "$base/service.plist" /Library/LaunchDaemons/dev.codex.onegram-keepalive.plist
"$base/enable.sh"
launchctl list dev.codex.onegram-keepalive
```

Registration must report `registered=1`. Open Onegram manually and confirm its icon and existing account still work. If the metadata change prevents launch, restore the app's saved plist and use the same scoped registration tool; do not clear all icon caches. Do not continue to claim success from a registration or launch return code alone.

After successful registration, remove the temporary privileged registration executable and its entitlement file; they are not needed for background operation:

```sh
rm -f /var/local/onegram-keepalive/register-onegram \
  /var/local/onegram-keepalive/registration-entitlements.plist
```

All payloads remain under `/var/local/onegram-keepalive` on the data partition. The LaunchDaemon and two Substrate entries at their standard system paths are small links. The service's logs are in a separate root-owned directory; app-side metadata logs rotate at 64 KiB. No message contents are logged.

## Verify actual delivery

1. Confirm `launchctl list dev.codex.onegram-keepalive` reports a running service. Its `daemon-logs/service.log` should show `assertion_valid=1` for Onegram's PID, and its `heartbeat` should be current and match that PID.
2. Open Onegram, press Home, and send it a message from another account/device. Confirm an **iOS notification outside Onegram**, not merely the app's own banner.
3. Lock the phone, leave Onegram in the app switcher, wait at least two minutes, and send another message without unlocking. Repeat with a longer locked interval.
4. Check the icon and account again. An in-app banner, enabled preferences, a fresh heartbeat, or one short test alone does not establish sustained locked-screen delivery.

In this case, the user confirmed Home Screen and lock-screen delivery and later reported it still working after 20 minutes. No periodic Telegram message polling, Wi-Fi-policy changes, transport resets, or speculative network-recovery patches were added after that confirmation. The exact duration is a user-observed test, not an automated long-term availability measurement.

## Disable or enable later

From a trusted root shell:

```sh
/var/local/onegram-keepalive/disable.sh
# Or, to enable again:
/var/local/onegram-keepalive/enable.sh
```

Reopen Onegram after toggling so its app hook loads or unloads. Disabling unloads the root service, removes its heartbeat and Substrate links, and preserves Onegram's icon, account, files, and repaired notification declaration. The private heartbeat gate also makes the loaded hook revert to the app's original pause behavior when the helper is absent or stale.

For complete helper removal, disable it first and remove only its LaunchDaemon link; preserve the plist backup before deleting the helper directory. Do not delete the app or restore a whole-phone notification preferences file. Incoming notifications may again stop once iOS suspends Onegram without the keep-alive service.
