#!/bin/sh
set -eu
task_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
python3 "$task_dir/validate_catalog.py"
mkdir -p "$task_dir/build"
compiler=${CC:-/usr/bin/clang}
for target in OpenAppStore register-store; do
  if [ "$target" = OpenAppStore ]; then source_file=store.c; else source_file=register-store.c; fi
  "$compiler" -target armv7-apple-ios9.0 -fPIE -fblocks -Wall -Wextra -Wno-unused-function \
    -fno-stack-protector -nostdlib -Wl,-ld_classic -Wl,-pie -Wl,-undefined,dynamic_lookup \
    -F"$task_dir/stubs" -framework UIKit -L"$task_dir/stubs" -lSystem \
    "$task_dir/src/$source_file" -o "$task_dir/build/$target"
done
