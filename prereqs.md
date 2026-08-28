# Prerequisites (macOS)

Everything you need — `clang` (the C compiler), `lldb` (the debugger), and
`make` — ships free with Apple's **Xcode Command Line Tools**. You do *not*
need the full Xcode app.

## Check if you already have them

```sh
xcode-select -p
```

If that prints a path like `/Library/Developer/CommandLineTools` (or
`/Applications/Xcode.app/...`), you're set. Verify each tool:

```sh
clang --version   # expect "Apple clang version ..."
make --version    # expect "GNU Make 3.81" (old but fine for this project)
lldb --version    # expect "lldb-..."
```

## Install if missing

```sh
xcode-select --install
```

A dialog pops up; click Install. It's a few GB and takes a while. When it
finishes, re-run the three `--version` checks above.

If `xcode-select -p` errors even after installing, reset it:

```sh
sudo xcode-select --reset
```

## Smoke test

```sh
cd ~/Developer/tinylang
printf '#include <stdio.h>\nint main(void){printf("hello\\n");return 0;}\n' > /tmp/hello.c
clang /tmp/hello.c -o /tmp/hello && /tmp/hello
```

If that prints `hello`, your toolchain works. Delete the temp files and start
[Phase 0](docs/phases/00-c-fundamentals.md) — its first task walks through
what that command actually did.

## Notes

- macOS's `make` is GNU Make 3.81 (Apple ships an old version for licensing
  reasons). Everything in this repo works with it.
- Linux tutorials will mention `gcc` and `gdb`. On macOS, `gcc` is actually
  an alias for clang, and `lldb` replaces gdb — same ideas, slightly
  different command names. There's an official gdb→lldb translation table
  linked in Phase 0, task 0.10.
- `valgrind` (a memory checker you'll see recommended everywhere) does not
  work on modern macOS. The replacements: UndefinedBehaviorSanitizer
  (built into clang, already enabled in this repo's Makefile) and the macOS
  `leaks` tool. Phase 0 tasks 0.8 and 0.10 teach both. (AddressSanitizer is
  the usual third leg, but as of Aug 2026 its runtime deadlocks on startup
  under macOS 26.5 + Apple clang 17 — the Makefile header explains how to
  try re-enabling it after a toolchain update.)
