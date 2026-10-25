#!/bin/sh
rm -f /var/local/onegram-keepalive/enabled
launchctl unload /Library/LaunchDaemons/dev.codex.onegram-keepalive.plist 2>/dev/null || true
rm -f /var/local/onegram-keepalive/heartbeat
rm -f /Library/MobileSubstrate/DynamicLibraries/OnegramKeepAlive.dylib /Library/MobileSubstrate/DynamicLibraries/OnegramKeepAlive.plist
echo "Onegram helper disabled. Reopen Onegram to unload its hook. Notification settings and account data are preserved."
