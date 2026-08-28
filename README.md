# tinylang

A tiny programming language, built from scratch in C, by hand.

This repo is a **learning roadmap plus scaffolding**, not a finished project.
The goal is to understand how programming languages actually work by building
one: a lexer, a recursive-descent parser, a tree-walking interpreter, and then
a bytecode compiler and stack-based virtual machine.

> **"tinylang" is a placeholder name.** Once you understand the mechanics,
> rename the language, redesign its syntax, and add features that are yours.
> This repo is a vehicle for learning, not a language you have to keep as-is.

## Current status

Phase 0, task 0.2 (variables/arithmetic) just went green — `make p0-02`
passes. Tasks 0.3 through 0.10 are still untouched stubs. ROADMAP.md and all
phase docs (00–07) are fully written; only the Phase 0 exercises are actually
in progress.

## The one ground rule

**You write every line of implementation code yourself.** No AI-generated
code, no copy-paste from tutorials. Reading books, docs, and other people's
code to *understand* is encouraged — that's what the links in each phase doc
are for. But when your fingers hit the keyboard, the code that lands in `src/`
and `phase0/` is yours, typed and understood by you. If you can't explain a
line, delete it and rewrite it until you can.

## How this is organized

- **`ROADMAP.md`** — the full phase list with time estimates.
- **`prereqs.md`** — get clang, lldb, and make working on your Mac (10 minutes).
- **`docs/phases/`** — one file per phase, each broken into ~1–2 hour tasks.
  Every task says **what to build**, **why it matters** (what concept it
  teaches), and gives **links to read *before* you start** — all verified to
  be live and free.
- **`phase0/`** — C fundamentals exercises: stub files with function
  signatures for you to implement, plus an assert-based test per task.
- **`src/`** — the language itself. Right now it holds the Phase 1 lexer
  scaffold (headers, stubs, comments — no implementation).
- **`tests/`** — hand-rolled tests: plain `main()` + `assert()`, no framework.
  **A task is done when its test passes.**
- **`reference/`** — working reference solutions for Phase 0 and Phase 1
  **only**. See the warning below.

## How to work

1. Open the current phase file in `docs/phases/`.
2. Read the linked material for the task *first*. The links are short and
   chosen specifically for that task — don't skip them and reverse-engineer
   the test instead; the point is the concept, not the green checkmark.
3. Implement the stubs in `phase0/` or `src/`.
4. Run the task's test (see below). Green = done, commit, next task.

## Build and run

Everything builds with `clang` via the `Makefile` — no dependencies.

```sh
make p0-02      # build + run the test for Phase 0, task 0.2
make p0-08      # ... task 0.8, etc.
make p1-1       # build + run the test for Phase 1, task 1.1
make lexdump    # build the token-dump tool from task 1.5
make clean      # delete build artifacts
```

Tests compile with `-fsanitize=undefined`, so out-of-bounds writes and other
undefined behavior crash loudly with a report naming the exact line instead
of corrupting things silently. That's a feature — read the report.
(AddressSanitizer is deliberately off: its runtime currently deadlocks on
this macOS/clang combo — details in the `Makefile` header.)

## When you're stuck

In order:

1. **The phase doc.** Re-read the task's "why" and its links — the answer is
   usually in the assigned reading.
2. **The test file.** It's the precise definition of "done"; read the asserts.
3. **The debugger.** `lldb ./build/p0-05` — Phase 0 task 0.10 teaches this.
4. **Crafting Interpreters** (linked throughout) — the book this roadmap
   leans on for Phases 1–6.
5. **`reference/` — last resort.** It holds working solutions for Phases 0–1.
   Opening it first robs you of the struggle that makes the concept stick.
   Use it to *compare after* you have something working, or to get unstuck
   after a genuine 30+ minute wall — and even then, read the smallest piece
   that unblocks you, close it, and write your own version from memory.
