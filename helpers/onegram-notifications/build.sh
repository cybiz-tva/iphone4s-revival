#!/bin/sh
set -eu
task_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
compiler=${CC:-/usr/bin/clang}
"$compiler" -target armv7-apple-ios9.0 -fblocks -fno-stack-protector -nostdlib \
  -Wl,-ld_classic -Wl,-no_pie -Wl,-undefined,dynamic_lookup \
  -L"$task_dir/stubs" -lSystem "$task_dir/keeper.c" -o "$task_dir/keeper"
"$compiler" -target armv7-apple-ios9.0 -fno-stack-protector -nostdlib -dynamiclib \
  -Wl,-ld_classic -Wl,-undefined,dynamic_lookup \
  -L"$task_dir/stubs" -lSystem "$task_dir/network-hook.c" -o "$task_dir/OnegramKeepAlive.dylib"
"$compiler" -target armv7-apple-ios9.0 -fno-stack-protector -nostdlib \
  -Wl,-ld_classic -Wl,-no_pie -Wl,-undefined,dynamic_lookup \
  -L"$task_dir/stubs" -lSystem "$task_dir/register-onegram.c" -o "$task_dir/register-onegram"
echo 'Built ARMv7 binaries. See docs/07-onegram-notifications.md before device signing or installation.'
