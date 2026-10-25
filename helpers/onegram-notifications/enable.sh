#!/bin/sh
set -e
test -x /var/local/onegram-keepalive/keeper
for suffix in dylib plist; do
 path=/Library/MobileSubstrate/DynamicLibraries/OnegramKeepAlive.$suffix
 target=/var/local/onegram-keepalive/OnegramKeepAlive.$suffix
 if test -L "$path"; then
  test "$path" -ef "$target"
 elif test -e "$path"; then
  echo "Path already occupied: $path"; exit 2
 else
  ln -s "$target" "$path"
 fi
done
touch /var/local/onegram-keepalive/enabled
launchctl list dev.codex.onegram-keepalive >/dev/null 2>&1 || launchctl load /Library/LaunchDaemons/dev.codex.onegram-keepalive.plist
echo "Onegram helper enabled. Reopen Onegram to load its hook."
