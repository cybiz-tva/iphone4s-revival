# A terminal for connecting to a Linux server

## Working version

**MTerminal 1.4-5** (`com.officialscheduler.mterminal`) was available in the phone's cached BigBoss metadata. Its compatibility tags included iOS 7/8/9, and its executable contained ARMv7. The package depended on firmware >= 4.0 and `system-cmds`.

The current NewTerm 2 package required iOS 10; it was not installed as an iOS 9 solution.

The exact MTerminal download matched a hash already in the phone's package metadata. BigBoss's HTTPS certificate failed validation at the time. The historic HTTP package source was used **only with the previously recorded expected SHA-256**. A checksum obtained solely alongside a download over insecure HTTP is not independent authentication.

After transferring the verified package to the phone:

```sh
dpkg -i /var/tmp/MTerminal-1.4-5.deb
uicache
```

Successful checks in the session:

```text
com.officialscheduler.mterminal  1.4-5
Terminal-executable-present
OpenSSH_6.7p1, OpenSSL 0.9.8zg
```

Its Home Screen name is **Terminal**, and its URL scheme is `mterminal://`.

## Connect from the phone

In Terminal, substitute your own server and account:

```sh
ssh user@SERVER_HOST
# For a custom port:
ssh -p 2222 user@SERVER_HOST
```

Check the server host-key fingerprint against a trusted source. Enter the password directly on the phone; password characters do not echo. Prefer a non-root account and appropriate key authentication where available. Do not publish credentials or globally weaken a Linux server's SSH configuration to accommodate this older client.

A test in this session established TCP connectivity to port 22 but timed out during SSH banner exchange. That test did not reach password authentication. The user later said they were already using the terminal. This is not proof that every modern SSH server's algorithm policy works with OpenSSH 6.7.

For a connection that is already open, `whoami` and `hostname` are simple checks of the remote shell. They were suggested, not recorded as executed verification here.
