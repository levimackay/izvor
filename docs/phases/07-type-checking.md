# Phase 7 (stretch) — Basic static type checking

**Goal:** a checker that walks the AST *before execution* and rejects
programs like `1 + true` or `if (5) ...` with a clear compile-time error —
the difference between finding a bug when that line eventually runs (maybe
in production) and finding it the moment you hit save.

**Why:** this is the idea behind Swift, TypeScript, Rust — the languages
you actually want to work in (and the one Apple will interview you in).
Having built the *dynamic* version first (Phase 4's runtime type errors),
you're in the rare position of implementing both sides of the oldest
argument in programming and understanding the tradeoff from the inside.
A type checker is also your fourth-or-fifth AST walk, which by now should
feel like home: same traversal, new question — not "what value?" but
"what *kind* of value, provably, without running it?"

**Spine reading:** Crafting Interpreters doesn't cover static typing (Lox
is dynamic), so the spine shifts:

- [Cornell CS 3110 textbook, "Type Checking"](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) —
  a free university-course chapter that builds a checker for a small
  language (SimPL) remarkably close to tinylang. The code is OCaml, but
  read it for the *rules* — each rule translates to one `switch` case in C.
- The classic book here is Pierce's *Types and Programming Languages*
  (library, not free online) — strictly optional, mentioned so you know
  where the deep end is.

---

## Task 7.1 — The type language + annotations (~1.5h)

**Build:** a `Type` enum (`TYPE_NUM`, `TYPE_BOOL` — that's genuinely
enough), and syntax for annotating declarations: `let x: num = 5;`
(lexer: colon token; parser: optional annotation on `let` and on function
parameters/returns).

**Why:** types now exist in three places — source syntax, checker, runtime
tags — and keeping them aligned is the daily bread of compiler teams.
Designing annotation syntax is also pure language design: mandatory or
optional? Where may they appear? You'll never read a Swift type signature
the same way.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) — the first sections, through "A Type System for SimPL".

**Done when:** annotated programs parse (asserts on the AST carrying the
annotation); the checker doesn't exist yet.

---

## Task 7.2 — Check expressions (~1.5–2h)

**Build:** `Type check_expr(Node*, TypeEnv*)`: literals have obvious types;
arithmetic requires num+num→num; comparisons num+num→bool; `==` requires
matching sides; `!` and `and`/`or` require bools. Errors report *expected
vs found*, with enough position info to be humane.

**Why:** this is the essence of type checking — a bottom-up proof over the
tree, computing "the type this expression *would* have" without computing
its value. The typing rules you're transcribing (`num + num : num`) are the
same judgment notation (`env ⊢ e : t`) in the CS 3110 chapter — you're
implementing a small formal system, and it will feel surprisingly like
`eval` with types where values go. That similarity is a deep fact
(abstract interpretation), not a coincidence.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) — the typing rules and the checker implementation.

**Done when:** asserts show `1 + 2` checks as num, `1 + true` is rejected
with a good message, `(1 < 2) == true` checks as bool.

---

## Task 7.3 — Check statements, variables, and control flow (~1.5–2h)

**Build:** a `TypeEnv` (name→Type, chained like Phase 4's runtime
environments — same structure, compile-time citizen), checking: `let`
initializers match annotations, assignments match declared types,
`if`/`while` conditions must be bool (goodbye `if (5)` — your language just
became stricter than C), and scoping rules mirror the runtime exactly.

**Why:** the checker maintaining a parallel, compile-time model of the same
scoping the interpreter does at runtime is the key structural insight of
static analysis — two walks, one set of rules, and any disagreement between
them is a soundness bug. "Condition must be bool" is your first opinionated
design departure from C; write one sentence in the doc defending it.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) — the let-binding and if rules.

**Done when:** well-typed programs pass; each rule has a rejection test;
running the checker before `eval` means the interpreter's runtime type
errors literally cannot fire anymore (leave them in as asserts — belt and
suspenders, and a proof you understand *why* they're now unreachable).

---

## Task 7.4 — Function types + wiring it into the pipeline (~1.5–2h)

**Build:** function signatures (`fn f(a: num, b: num): num`), checking
calls (argument types and count against parameters, result type = declared
return) and bodies (every `return` matches the declared type). Then wire
the checker in: parse → **check** → eval/compile, with `--no-check` as an
escape hatch flag.

**Why:** function types are the first *compound* types — the `Type` enum
grows into a struct, and suddenly you see why real type systems are
recursive data structures (what's the type of a function returning a
function?). Wiring the pipeline gives you the full modern-language
architecture: parse, analyze, then run — the same stage list, in miniature,
as swiftc.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) — function/anonymous-function rules if present in your edition; otherwise derive the rule yourself from the call rule — at this point, you can.

**Done when:** typed fib checks and runs; a call with a bool argument to a
num parameter is rejected at compile time; the whole examples/ folder still
runs green through the full pipeline.

---

## After this

You've built, by hand, in C: a lexer, a parser, an AST, a tree-walking
interpreter, a bytecode compiler, a virtual machine, and a type checker.
That's the front-to-back anatomy of every language you'll ever use.

Now do the thing the README promised: **make it yours.** Rename it. Change
the syntax you never liked. Add strings, arrays, closures, a GC (Crafting
Interpreters ch. 19–26 cover all four). And put it on your résumé in plain
words: *designed and implemented a statically-typed programming language in
C — recursive-descent parser, bytecode compiler, stack-based VM* — then
enjoy being able to answer every follow-up question they ask.
