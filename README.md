# tinylang

A programming language, built from scratch in C, by hand.

I'm building this to actually understand how languages work instead of just using them: a lexer, a recursive descent parser, a tree walking interpreter, then a bytecode compiler and a stack based VM. "tinylang" is a placeholder name. Once the mechanics click I'll redesign the syntax and make it mine.

## Where it's at

Phase 0, task 0.5 (pointers) in progress. Tasks 0.2 through 0.4 (variables/arithmetic, control flow, arrays) are done and green. ROADMAP.md and all phase docs (00-07) are fully written; the exercises are what's actually in progress right now.

## The rule I hold myself to

I write every line of implementation code myself. No AI generated code, no copy paste from tutorials. I read books, docs, and other people's code to understand the concept, but the code that lands in `src/` and `phase0/` is mine, typed and understood by me. If I can't explain a line, I delete it and rewrite it until I can.

## Layout

- `ROADMAP.md` — full phase list with time estimates
- `docs/phases/` — one file per phase, broken into 1-2 hour tasks, each with what to build, why it matters, and links to read first
- `phase0/` — C fundamentals exercises, stub files plus an assert based test per task
- `src/` — the language itself; currently holds the Phase 1 lexer scaffold, no implementation yet
- `tests/` — hand rolled tests, plain `main()` + `assert()`, no framework
- `reference/` — working solutions for Phase 0 and Phase 1 only, last resort

## Build and run

Everything builds with `clang` via the Makefile, no dependencies.

```sh
make p0-02      # build + run the test for Phase 0, task 0.2
make p0-08      # ... task 0.8, etc.
make p1-1       # build + run the test for Phase 1, task 1.1
make lexdump    # build the token-dump tool from task 1.5
make clean      # delete build artifacts
```

Tests compile with `-fsanitize=undefined`, so undefined behavior crashes loudly with the exact line instead of failing silently. AddressSanitizer is off on purpose, its runtime deadlocks on this macOS/clang combo (details in the Makefile header).
